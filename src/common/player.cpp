#include "player.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include "networking/actions_packet.hpp"
#include "physics/util.hpp"

const float PLAYER_ROTATE_SPEED = 0.05f;
const glm::vec3 CAMERA_OFFSET = glm::vec3(0, 0.4f, 0);
constexpr float GRAVITY = 0.04f;
constexpr float PLAYER_JUMP_SPEED = 0.28f;
constexpr float PLAYER_COLLIDER_DIMENSION = 0.2f;
constexpr float PLAYER_DRAG = 0.03f;
constexpr float MAX_PLAYER_SPEED = 0.2f;

player::player(unsigned int id, const std::string& name)
	: _id(id), _name(name), _size(0.5f, 0.5f, 0.5f), _color(0.1, 0.1, 0.4), _on_left_mouse_pressed(false), _on_right_mouse_pressed(false)
{}

player::player(unsigned int id, const std::string& name, const glm::vec3& position)
	: _id(id), _name(name), _position(position), _size(0.5f, 0.5f, 0.5f), _color(0.02, 0.02, 0.2), _on_left_mouse_pressed(false), _on_right_mouse_pressed(false)
{}

unsigned int player::get_id() const {
	return _id;
}

const std::string& player::get_name() const {
	return _name;
}

const glm::vec3& player::get_position() const {
	return _position;
}

const glm::vec2& player::get_view_angles() const {
	return _view_angles;
}

const glm::vec3& player::get_speed() const {
	return _speed;
}

std::uint8_t player::get_actions() const {
	return _actions;
}

bool player::poll_left_mouse_pressed() {
	bool lmp = _on_left_mouse_pressed;
	_on_left_mouse_pressed = false;
	return lmp;
}

bool player::poll_right_mouse_pressed() {
	bool rmp = _on_right_mouse_pressed;
	_on_right_mouse_pressed = false;
	return rmp;
}

glm::vec3 player::get_color() const {
	return _color;
}

void player::set_name(const std::string& name) {
	_name = name;
}

void player::set_position(const glm::vec3& position) {
	_position = position;
}

void player::set_view_angles(const glm::vec2& view_angles) {
	_view_angles = view_angles;
}

void player::set_speed(const glm::vec3& speed) {
	_speed = speed;
}

void player::set_actions(const std::uint8_t actions) {
	if (actions & LEFT_MOUSE_PRESSED && !(_actions & LEFT_MOUSE_PRESSED)) {
		_on_left_mouse_pressed = true;
	}
	if (actions & RIGHT_MOUSE_PRESSED && !(_actions & RIGHT_MOUSE_PRESSED)) {
		_on_right_mouse_pressed = true;
	}
	_actions = actions;
}

void player::update_direction(const glm::vec2& direction_update) {
	_view_angles.y += direction_update.x * PLAYER_ROTATE_SPEED;
	_view_angles.x -= direction_update.y * PLAYER_ROTATE_SPEED;
	_view_angles.x = fmax(fmin(_view_angles.x, 89.f), -89.0f);
}

glm::vec3 player::get_up() {
	return glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 player::get_right() const {
	return glm::normalize(glm::cross(get_direction(), player::get_up()));
}

glm::vec3 player::get_direction() const {
	return glm::normalize(glm::vec3(
				cos(glm::radians(_view_angles.x)) * cos(glm::radians(_view_angles.y)),
				sin(glm::radians(_view_angles.x)),
				cos(glm::radians(_view_angles.x)) * sin(glm::radians(_view_angles.y))
			));
}

glm::vec3 player::get_top() const {
	return glm::normalize(glm::cross(get_right(), get_direction()));
}

glm::mat4 player::get_look_at() const {
	return glm::lookAt(get_camera_position(), _position + CAMERA_OFFSET + get_direction(), get_up());
	// return glm::lookAt(_position - get_direction()*5.f, _position, get_up());
}

glm::vec3 player::get_camera_position() const {
	return _position + CAMERA_OFFSET;
}

void player::respawn(const glm::vec3& position) {
	_position = position;
	_speed = glm::vec3();
	_view_angles = glm::vec2();
}

bool player::tick(const block_container& blocks) {
	_speed.y -= GRAVITY;
	apply_player_movements(blocks);
	_position += _speed;
	return physics(blocks);
}

void player::apply_player_movements(const block_container& blocks) {
	float forward = 0;
	if (_actions & FORWARD_ACTION)
		forward++;
	if (_actions & BACKWARD_ACTION)
		forward--;

	float right = 0;
	if (_actions & LEFT_ACTION)
		right--;
	if (_actions & RIGHT_ACTION)
		right++;

	if (_actions & JUMP_ACTION) {
		if (!blocks.get_colliding_blocks(get_bottom_collider()).empty()) {
			_speed = get_up() * PLAYER_JUMP_SPEED;
		}
	}

	glm::vec3 tmp_direction = get_direction();
	tmp_direction.y = 0.f;

	_speed += (get_right()*right + tmp_direction*forward)*0.1f;

	glm::vec2 tmp_speed = glm::vec2(_speed.x, _speed.z);

	if (glm::length(tmp_speed) <= PLAYER_DRAG) {
		tmp_speed = glm::vec3();
	} else {
		tmp_speed += glm::normalize(tmp_speed) * -PLAYER_DRAG;
		if (glm::length(tmp_speed) > MAX_PLAYER_SPEED) {
			tmp_speed *= MAX_PLAYER_SPEED / glm::length(tmp_speed);
		}
	}
	_speed.x = tmp_speed.x;
	_speed.z = tmp_speed.y;
}

bool player::physics(const block_container& blocks) {
	check_collider(blocks, get_left_collider()  , -1, 2);
	check_collider(blocks, get_right_collider() ,  1, 2);
	check_collider(blocks, get_back_collider()  , -1, 0);
	check_collider(blocks, get_front_collider() ,  1, 0);
	check_collider(blocks, get_bottom_collider(), -1, 1);
	check_collider(blocks, get_top_collider()   ,  1, 1);

	bool was_winning = false;

	for (const world_block& wb : blocks.get_colliding_blocks(get_bottom_collider())) {
		if (wb.is_winning_block()) {
			respawn(blocks.get_respawn_position()); // TODO: Implement greater winning reward
			was_winning = true;
		}
	}

	if (_position.y < blocks.get_min_y() - 100.f) {
		respawn(blocks.get_respawn_position());
	}

	return was_winning;
}

// direction = -1, if block is in negative direction to player
void player::check_collider(const block_container& blocks, const cuboid& collider, int direction, unsigned int coordinate) {
	std::vector<world_block> colliding_blocks = blocks.get_colliding_blocks(collider);
	if (!colliding_blocks.empty()) {
		if (_speed[coordinate]*direction > 0.f) {
			_speed[coordinate] = 0.f;
		}
		float min_coord = colliding_blocks[0].get_position()[coordinate]*direction;
		for (const world_block& wb : colliding_blocks) {
			min_coord = glm::min(min_coord, static_cast<float>(wb.get_position()[coordinate]*direction));
		}
		_position[coordinate] = (min_coord*direction) - (0.5f + _size.y - 0.01f)*direction;
	}
}

cuboid player::get_bottom_collider() const {
	return cuboid(
		glm::vec3(_position.x, _position.y-0.4f, _position.z),
		glm::vec3(PLAYER_COLLIDER_DIMENSION, 0.1f, PLAYER_COLLIDER_DIMENSION)
	);
}

cuboid player::get_top_collider() const {
	return cuboid(
		glm::vec3(_position.x, _position.y+0.4f, _position.z),
		glm::vec3(PLAYER_COLLIDER_DIMENSION, 0.1f, PLAYER_COLLIDER_DIMENSION)
	);
}

cuboid player::get_left_collider() const {
	return cuboid(
		glm::vec3(_position.x, _position.y, _position.z-0.4f),
		glm::vec3(PLAYER_COLLIDER_DIMENSION, PLAYER_COLLIDER_DIMENSION, 0.1f)
	);
}

cuboid player::get_right_collider() const {
	return cuboid(
		glm::vec3(_position.x, _position.y, _position.z+0.4f),
		glm::vec3(PLAYER_COLLIDER_DIMENSION, PLAYER_COLLIDER_DIMENSION, 0.1f)
	);
}

cuboid player::get_front_collider() const {
	return cuboid(
		glm::vec3(_position.x+0.4f, _position.y, _position.z),
		glm::vec3(0.1f, PLAYER_COLLIDER_DIMENSION, PLAYER_COLLIDER_DIMENSION)
	);
}

cuboid player::get_back_collider() const {
	return cuboid(
		glm::vec3(_position.x-0.4f, _position.y, _position.z),
		glm::vec3(0.1f, PLAYER_COLLIDER_DIMENSION, PLAYER_COLLIDER_DIMENSION)
	);
}
