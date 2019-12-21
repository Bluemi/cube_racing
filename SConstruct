#!/usr/bin/python3

import os
import shutil
import hashlib

DRY=False

SRC_DIRECTORY = 'src'
BUILD_DIRECTORY = 'build'
OBJ_DIRECTORY = os.path.join(BUILD_DIRECTORY, 'obj')
BIN_DIRECTORY = os.path.join(BUILD_DIRECTORY, 'bin')

def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.digest()

def file_exists(src_file, dest_file):
    if not os.path.isfile(dest_file):
        return False

    return md5(src_file) == md5(dest_file)


def is_source_file(f):
    return os.path.splitext(f)[1] in ['.hpp', '.cpp', '.fs', '.vs']


def copy_tree(src_directory, destination_directory, dry=False):
    """
    Copies the given src directory to the destination directory.
    Files that already exists with the same version at the destination directory are ignored.
    """
    for src_root, directories, files in os.walk(src_directory):
        build_root = os.path.join(destination_directory, src_root[len(src_directory)+1:])
        if dry:
            if not os.path.isdir(build_root):
                print('mkdir {}'.format(build_root))
        else:
            os.makedirs(build_root, exist_ok=True)

        # create directories
        for d in directories:
            current_src_dir = os.path.join(src_root, d)
            current_dest_dir = os.path.join(build_root, d)

            if dry:
                if not os.path.isdir(current_dest_dir):
                    print('mkdir {}'.format(current_dest_dir))
            else:
                if not os.path.isdir(current_dest_dir):
                    os.makedirs(current_dest_dir)

        # copy files
        for f in files:
            current_src_file = os.path.join(src_root, f)
            current_dest_file = os.path.join(build_root, f)

            if not file_exists(current_src_file, current_dest_file):
                if dry:
                    print('cp {} -> {}'.format(current_src_file, current_dest_file))
                else:
                    shutil.copy2(current_src_file, current_dest_file)


        # remove files
        build_files = [f for f in os.listdir(build_root) if os.path.isfile(os.path.join(build_root, f))]
        src_files = [f for f in os.listdir(src_root) if os.path.isfile(os.path.join(build_root, f))]
        for build_file in build_files:
            if not is_source_file(build_file):
                continue
            if build_file not in src_files:
                if dry:
                    print('rm {}'.format(os.path.join(build_root, build_file)))
                else:
                    os.remove(os.path.join(build_root, build_file))

copy_tree(SRC_DIRECTORY, OBJ_DIRECTORY, dry=DRY)


def get_server_source_files(env, exclude=[]):
    dirs = []
    for root, _, _ in os.walk(OBJ_DIRECTORY):
        dirs.append(root)

    source_files = []
    for d in dirs:
        excludes = [os.path.join(d, e) for e in exclude]
        source_files.extend(env.Glob(os.path.join(d, '*.cpp'), exclude=excludes))

    return source_files


env = Environment(parse_flags='-lboost_thread.a -std=c++17')
env['ENV']['TERM'] = os.environ['TERM']
env.Append(LIBS=['pthread', 'GLU', 'glfw3', 'X11', 'Xxf86vm', 'Xrandr', 'pthread', 'Xi', 'dl', 'Xinerama', 'Xcursor']),
# env.Append(CCFLAGS='-O3')

server_source_files = get_server_source_files(env, exclude=['client.cpp'])
client_source_files = get_server_source_files(env, exclude=['server.cpp'])
if DRY:
    print('server source files:')
    for server_source_file in server_source_files:
        print('\t{}'.format(server_source_file.name))
    print('client source files:')
    for client_source_file in client_source_files:
        print('\t{}'.format(client_source_file.name))
else:
    env.Program(os.path.join(BIN_DIRECTORY, 'server'), server_source_files)
    env.Program(os.path.join(BIN_DIRECTORY, 'client'), client_source_files)
