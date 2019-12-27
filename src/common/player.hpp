#ifndef __PLAYER_CLASS__
#define __PLAYER_CLASS__

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "world/world_block.hpp"

class shader_program;

class player {
	public:
		player(unsigned int id, const std::string& name);
		player(unsigned int id, const std::string& name, const glm::vec3& position);

		unsigned int get_id() const;
		const std::string& get_name() const;
		const glm::vec3& get_position() const;
		const glm::vec2& get_view_angles() const;
		const glm::vec3& get_speed() const;
		std::uint8_t get_actions() const;

		void set_name(const std::string& name);
		void set_position(const glm::vec3& position);
		void set_view_angles(const glm::vec2& view_angles);
		void set_speed(const glm::vec3& speed);
		void set_actions(const std::uint8_t actions);
		void update_direction(const glm::vec2& direction_update);

		static glm::vec3 get_up();
		glm::vec3 get_right() const;
		glm::vec3 get_direction() const;
		glm::vec3 get_top() const;
		glm::mat4 get_look_at() const;

		void tick(const block_container& blocks);
		void apply_player_movements();
		void physics(const block_container& blocks);

		cuboid get_bottom_collider() const;
		cuboid get_top_collider() const;
		cuboid get_left_collider() const;
		cuboid get_right_collider() const;
		cuboid get_front_collider() const;
		cuboid get_back_collider() const;
	private:
		char _id;
		std::string _name;
		glm::vec3 _position;
		glm::vec2 _view_angles;
		glm::vec3 _speed;
		std::uint8_t _actions;

};

#endif
