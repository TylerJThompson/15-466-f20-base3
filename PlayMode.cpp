#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <cmath>

GLuint platformer_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > platformer_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("platformer.pnct"));
	platformer_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > platformer_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("platformer.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = platformer_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = platformer_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > bar_brawl_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("bar-brawl.opus"));
});

Load< Sound::Sample > bflat_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-bflat.opus"));
});

Load< Sound::Sample > c_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-c.opus"));
});

Load< Sound::Sample > d_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-d.opus"));
});

Load< Sound::Sample > eflat_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-eflat.opus"));
});

Load< Sound::Sample > f_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-f.opus"));
});

Load< Sound::Sample > g_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-g.opus"));
});

Load< Sound::Sample > a_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-a.opus"));
});

Load< Sound::Sample > full_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("my-game3-full.opus"));
});

PlayMode::PlayMode() : scene(*platformer_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		//referenced for checking is a string includes a substring: https://stackoverflow.com/questions/2340281/check-if-a-string-contains-a-string-in-c
		if (transform.name.find("final") != std::string::npos) collect_locations.push_back(&transform);
		if (transform.name.find("ground") != std::string::npos) ground_transforms.push_back(&transform);
		else if (transform.name == "jewel.001") bflat.transform = &transform;
		else if (transform.name == "jewel.002") c.transform = &transform;
		else if (transform.name == "jewel.003") d.transform = &transform;
		else if (transform.name == "jewel.004") eflat.transform = &transform;
		else if (transform.name == "jewel.005") f.transform = &transform;
		else if (transform.name == "jewel.006") g.transform = &transform;
		else if (transform.name == "jewel.007") a.transform = &transform;
	}
	if (bflat.transform == nullptr) throw std::runtime_error("B-Flat collectible not found.");
	if (c.transform == nullptr) throw std::runtime_error("C collectible not found.");
	if (d.transform == nullptr) throw std::runtime_error("D collectible not found.");
	if (eflat.transform == nullptr) throw std::runtime_error("E-Flat collectible not found.");
	if (f.transform == nullptr) throw std::runtime_error("F collectible not found.");
	if (g.transform == nullptr) throw std::runtime_error("G collectible not found.");
	if (a.transform == nullptr) throw std::runtime_error("A collectible not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	bgm = Sound::play_3D(*full_sample, 1.0f, ground_transforms.front()->position, 1.0f);
	time_since_start = 0.0f;
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			//this section below lets the camera move based on mouse motion, but never about the y-axis
			camera->transform->rotation
				*= glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				*= glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::vec3 euler = quaternion_to_euler(camera->transform->rotation);
			euler.x = glm::min(euler.x, 150.0f);
			euler.x = glm::max(euler.x, 30.0f);
			euler.y = 0.0f;
			camera->transform->rotation = euler_to_quaternion(euler);
			camera->transform->rotation = glm::normalize(camera->transform->rotation);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//move sound to follow leg tip position:
	//leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);*/

	//playing music and notes at start then placing the collectibles:
	if (in_start_sequence) {
		time_since_start += elapsed;
		if (time_since_start >= 4.0f && bflat.note == nullptr) {
			bflat.index = 0;
			bflat.transform->position.z += 0.25f;
			bflat.note = Sound::play_3D(*bflat_sample, 1.0f, bflat.transform->position, 1.0f);
		} else if (time_since_start >= 6.0f && a.note == nullptr) {
			a.index = 1;
			a.transform->position.z += 0.25f;
			bflat.transform->position.z -= 0.25f;
			a.note = Sound::play_3D(*a_sample, 1.0f, a.transform->position, 1.0f);
		} else if (time_since_start >= 8.0f && g.note == nullptr) {
			g.index = 2;
			g.transform->position.z += 0.25f;
			a.transform->position.z -= 0.25f;
			g.note = Sound::play_3D(*g_sample, 1.0f, g.transform->position, 1.0f);
		} else if (time_since_start >= 10.0f && f.note == nullptr) {
			f.index = 3;
			f.transform->position.z += 0.25f;
			g.transform->position.z -= 0.25f;
			f.note = Sound::play_3D(*f_sample, 1.0f, f.transform->position, 1.0f);
		} else if (time_since_start >= 12.0f && eflat.note == nullptr) {
			eflat.index = 4;
			eflat.transform->position.z += 0.25f;
			f.transform->position.z -= 0.25f;
			eflat.note = Sound::play_3D(*eflat_sample, 1.0f, eflat.transform->position, 1.0f);
		} else if (time_since_start >= 14.0f && c.note == nullptr) {
			c.index = 5;
			c.transform->position.z += 0.25f;
			eflat.transform->position.z -= 0.25f;
			c.note = Sound::play_3D(*c_sample, 1.0f, c.transform->position, 1.0f);
		}
		else if (time_since_start >= 16.0f && d.note == nullptr) {
			d.index = 6;
			d.transform->position.z += 0.25f;
			c.transform->position.z -= 0.25f;
			d.note = Sound::play_3D(*d_sample, 1.0f, d.transform->position, 1.0f);
		}
		else if (time_since_start >= 18.0f) {
			d.transform->position.z -= 0.25f;
			if (played_start_once) {
				uint8_t i = 0;
				for (auto transform : collect_locations) {
					std::cout << i << std::endl;
					if (i == bflat.index) {
						bflat.transform->position = transform->position + glm::vec3(0.0f, 0.0f, 0.1f);
						bflat.note = Sound::loop_3D(*bflat_sample, 1.0f, bflat.transform->position, 1.0f);
					} else if (i == a.index) {
						a.transform->position = transform->position + glm::vec3(0.0f, 0.0f, 0.1f);
						//a.note = Sound::loop_3D(*a_sample, 1.0f, a.transform->position, 1.0f);
					} else if (i == g.index) {
						g.transform->position = transform->position + glm::vec3(0.0f, 0.0f, 0.1f);
						//g.note = Sound::loop_3D(*g_sample, 1.0f, g.transform->position, 1.0f);
					} else if (i == f.index) {
						f.transform->position = transform->position + glm::vec3(0.0f, 0.0f, 0.1f);
						//f.note = Sound::loop_3D(*f_sample, 1.0f, f.transform->position, 1.0f);
					} else if (i == eflat.index) {
						eflat.transform->position = transform->position + glm::vec3(0.0f, 0.0f, 0.1f);
						//eflat.note = Sound::loop_3D(*bflat_sample, 1.0f, eflat.transform->position, 1.0f);
					} else if (i == c.index) {
						c.transform->position = transform->position + glm::vec3(0.0f, 0.0f, 0.1f);
						//c.note = Sound::loop_3D(*c_sample, 1.0f, c.transform->position, 1.0f);
					} else {
						d.transform->position = transform->position + glm::vec3(0.0f, 0.0f, 0.1f);
						//d.note = Sound::loop_3D(*d_sample, 1.0f, d.transform->position, 1.0f);
					}
					i += 1;
				}
				in_start_sequence = false;
			} else {
				bflat.note = nullptr;
				a.note = nullptr;
				g.note = nullptr;
				f.note = nullptr;
				eflat.note = nullptr;
				c.note = nullptr;
				d.note = nullptr;
				bgm = Sound::play_3D(*full_sample, 1.0f, ground_transforms.front()->position, 1.0f);
				time_since_start = 0.0f;
				played_start_once = true;
			}
		}
	}
	//check for collisions with collectibles and track progress towards finish
	else {
		glm::vec3 cam_pos = camera->transform->position;
		glm::vec3 collect_pos;
		if (collect_index == bflat.index) {
			collect_pos = bflat.transform->position;
			if (cam_pos.x >= collect_pos.x - 0.5f && cam_pos.x <= collect_pos.x + 0.5f &&
				cam_pos.y >= collect_pos.y - 0.5f && cam_pos.y <= collect_pos.y + 0.5f &&
				cam_pos.z >= collect_pos.z - 0.5f && cam_pos.z <= collect_pos.z + 0.5f) {
				bflat.transform->position = glm::vec3(-1.5f, 0.0f, 0.3f);
				Sound::stop_all_samples();
				a.note = Sound::loop_3D(*a_sample, 1.0f, a.transform->position, 1.0f);
				collect_index += 1;
			}
		}
		else if (collect_index == a.index) {
			collect_pos = a.transform->position;
			if (cam_pos.x >= collect_pos.x - 0.5f && cam_pos.x <= collect_pos.x + 0.5f &&
				cam_pos.y >= collect_pos.y - 0.5f && cam_pos.y <= collect_pos.y + 0.5f &&
				cam_pos.z >= collect_pos.z - 0.5f && cam_pos.z <= collect_pos.z + 0.5f) {
				a.transform->position = glm::vec3(-1.0f, 0.0f, 0.3f);
				Sound::stop_all_samples();
				g.note = Sound::loop_3D(*g_sample, 1.0f, g.transform->position, 1.0f);
				collect_index += 1;
			}
		}
		else if (collect_index == g.index) {
			collect_pos = g.transform->position;
			if (cam_pos.x >= collect_pos.x - 0.5f && cam_pos.x <= collect_pos.x + 0.5f &&
				cam_pos.y >= collect_pos.y - 0.5f && cam_pos.y <= collect_pos.y + 0.5f &&
				cam_pos.z >= collect_pos.z - 0.5f && cam_pos.z <= collect_pos.z + 0.5f) {
				g.transform->position = glm::vec3(-0.5f, 0.0f, 0.3f);
				Sound::stop_all_samples();
				f.note = Sound::loop_3D(*f_sample, 1.0f, f.transform->position, 1.0f);
				collect_index += 1;
			}
		}
		else if (collect_index == f.index) {
			collect_pos = f.transform->position;
			if (cam_pos.x >= collect_pos.x - 0.5f && cam_pos.x <= collect_pos.x + 0.5f &&
				cam_pos.y >= collect_pos.y - 0.5f && cam_pos.y <= collect_pos.y + 0.5f &&
				cam_pos.z >= collect_pos.z - 0.5f && cam_pos.z <= collect_pos.z + 0.5f) {
				f.transform->position = glm::vec3(-0.0f, 0.0f, 0.3f);
				Sound::stop_all_samples();
				eflat.note = Sound::loop_3D(*eflat_sample, 1.0f, eflat.transform->position, 1.0f);
				collect_index += 1;
			}
		}
		else if (collect_index == eflat.index) {
			collect_pos = eflat.transform->position;
			if (cam_pos.x >= collect_pos.x - 0.5f && cam_pos.x <= collect_pos.x + 0.5f &&
				cam_pos.y >= collect_pos.y - 0.5f && cam_pos.y <= collect_pos.y + 0.5f &&
				cam_pos.z >= collect_pos.z - 0.5f && cam_pos.z <= collect_pos.z + 0.5f) {
				eflat.transform->position = glm::vec3(0.5f, 0.0f, 0.3f);
				Sound::stop_all_samples();
				c.note = Sound::loop_3D(*c_sample, 1.0f, c.transform->position, 1.0f);
				collect_index += 1;
			}
		}
		else if (collect_index == c.index) {
			collect_pos = c.transform->position;
			if (cam_pos.x >= collect_pos.x - 0.5f && cam_pos.x <= collect_pos.x + 0.5f &&
				cam_pos.y >= collect_pos.y - 0.5f && cam_pos.y <= collect_pos.y + 0.5f &&
				cam_pos.z >= collect_pos.z - 0.5f && cam_pos.z <= collect_pos.z + 0.5f) {
				c.transform->position = glm::vec3(1.0f, 0.0f, 0.3f);
				Sound::stop_all_samples();
				d.note = Sound::loop_3D(*d_sample, 1.0f, d.transform->position, 1.0f);
				collect_index += 1;
			}
		}
		else {
			collect_pos = d.transform->position;
			if (cam_pos.x >= collect_pos.x - 0.5f && cam_pos.x <= collect_pos.x + 0.5f &&
				cam_pos.y >= collect_pos.y - 0.5f && cam_pos.y <= collect_pos.y + 0.5f &&
				cam_pos.z >= collect_pos.z - 0.5f && cam_pos.z <= collect_pos.z + 0.5f) {
				d.transform->position = glm::vec3(1.5f, 0.0f, 0.3f);
				Sound::stop_all_samples();
				bgm = Sound::loop_3D(*full_sample, 1.0f, ground_transforms.front()->position, 10.0f);
				winner = true;
			}
		}
	}

	//rotate collectibles
	{
		//constexpr float CollectibleBounce = 3.0f;
		constexpr float CollectibleRotate = 75.0f;

		//B-Flat
		glm::vec3 bflat_euler = quaternion_to_euler(bflat.transform->rotation);
		bflat_euler.z += CollectibleRotate * elapsed;
		if (bflat_euler.z >= 360.0f)
			bflat_euler.z -= 360.0f;
		bflat.transform->rotation = euler_to_quaternion(bflat_euler);

		//C
		glm::vec3 c_euler = quaternion_to_euler(c.transform->rotation);
		c_euler.z += CollectibleRotate * elapsed;
		if (c_euler.z >= 360.0f)
			c_euler.z -= 360.0f;
		c.transform->rotation = euler_to_quaternion(c_euler);

		//D
		glm::vec3 d_euler = quaternion_to_euler(d.transform->rotation);
		d_euler.z += CollectibleRotate * elapsed;
		if (d_euler.z >= 360.0f)
			d_euler.z -= 360.0f;
		d.transform->rotation = euler_to_quaternion(d_euler);

		//E-Flat
		glm::vec3 eflat_euler = quaternion_to_euler(eflat.transform->rotation);
		eflat_euler.z += CollectibleRotate * elapsed;
		if (eflat_euler.z >= 360.0f)
			eflat_euler.z -= 360.0f;
		eflat.transform->rotation = euler_to_quaternion(eflat_euler);

		//F
		glm::vec3 f_euler = quaternion_to_euler(f.transform->rotation);
		f_euler.z += CollectibleRotate * elapsed;
		if (f_euler.z >= 360.0f)
			f_euler.z -= 360.0f;
		f.transform->rotation = euler_to_quaternion(f_euler);

		//G
		glm::vec3 g_euler = quaternion_to_euler(g.transform->rotation);
		g_euler.z += CollectibleRotate * elapsed;
		if (g_euler.z >= 360.0f)
			g_euler.z -= 360.0f;
		g.transform->rotation = euler_to_quaternion(g_euler);

		//A
		glm::vec3 a_euler = quaternion_to_euler(a.transform->rotation);
		a_euler.z += CollectibleRotate * elapsed;
		if (a_euler.z >= 360.0f)
			a_euler.z -= 360.0f;
		a.transform->rotation = euler_to_quaternion(a_euler);
	}

	//move camera:
	{
		//move camera back to center if player fell:
		if (camera->transform->position.z <= -10.0f)
			camera->transform->position = ground_transforms.front()->position + glm::vec3(0.0f, 0.0f, 0.6f);

		//the minimum height the player should be able to fall:
		float min_height = -10.0f;

		//check if camera is above a ground panel
		glm::vec3 cam_pos = camera->transform->position;
		for (auto transform : ground_transforms) {
			glm::vec3 ground_pos = transform->position;
			glm::vec3 ground_scale = transform->scale;
			if (cam_pos.x >= ground_pos.x - ground_scale.x && cam_pos.x <= ground_pos.x + ground_scale.x &&
				cam_pos.y >= ground_pos.y - ground_scale.y && cam_pos.y <= ground_pos.y + ground_scale.y &&
				cam_pos.z >= ground_pos.z + 0.35f) {
				min_height = ground_pos.z + 0.6f;
				break;
			}
		}

		//how fast the player moves
		constexpr float PlayerSpeed = 3.0f;
		
		//combine inputs into a move:
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x = -1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y = -1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;
		if (space.pressed && cam_pos.z == min_height && jump_velocity == 0.0f) jump_velocity = 2.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_world();
		glm::vec3 right = frame[0];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
		//camera->transform->position.z = 0.6f;

		//make camera jump and fall
		if (camera->transform->position.z < min_height - 0.05f)
			jump_velocity = 0.0f;
		if (jump_velocity > 0.0f) {
			camera->transform->position.z += (jump_velocity - 1.0f) * PlayerSpeed * elapsed;
			jump_velocity = glm::max(jump_velocity - elapsed, 0.0f);
		}
		else if (camera->transform->position.z > min_height)
			camera->transform->position.z = glm::max(cam_pos.z - PlayerSpeed * elapsed, min_height);
		else if (camera->transform->position.z < min_height)
			camera->transform->position.z = min_height;
	}

	//update listener to camera position:
	{
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	GL_ERRORS();
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		//drawing text on the screen:
		constexpr float H = 0.09f;
		float ofs = 2.0f / drawable_size.y;
		if (winner) {
			lines.draw_text("You win!",
				glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			lines.draw_text("You win!",
				glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		} else {
			lines.draw_text("Mouse motion rotates camera; WASD moves; space bar jumps; escape ungrabs mouse",
				glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			lines.draw_text("Mouse motion rotates camera; WASD moves; space bar jumps; escape ungrabs mouse",
				glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		}
	}
}

//this function came from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
glm::quat PlayMode::euler_to_quaternion(glm::vec3 euler) {
	//convert degrees to radians from http://cplusplus.com/forum/beginner/144006/
	glm::vec3 radians;
	radians.x = (euler.x * (float)M_PI) / 180.0f;
	radians.y = (euler.y * (float)M_PI) / 180.0f;
	radians.z = (euler.z * (float)M_PI) / 180.0f;
	
	//abbreviations for the various angular functions
	double cy = cos(radians.z * 0.5);
	double sy = sin(radians.z * 0.5);
	double cp = cos(radians.y * 0.5);
	double sp = sin(radians.y * 0.5);
	double cr = cos(radians.x * 0.5);
	double sr = sin(radians.x * 0.5);

	//get the quaternion to return
	glm::quat quaternion;
	quaternion.w = float(cr * cp * cy + sr * sp * sy);
	quaternion.x = float(sr * cp * cy - cr * sp * sy);
	quaternion.y = float(cr * sp * cy + sr * cp * sy);
	quaternion.z = float(cr * cp * sy - sr * sp * cy);

	return quaternion;
}

//this function came from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
glm::vec3 PlayMode::quaternion_to_euler(glm::quat quaternion) {
	glm::vec3 euler;
	
	// roll (x-axis rotation)
	double sinr_cosp = 2 * (quaternion.w * quaternion.x + quaternion.y * quaternion.z);
	double cosr_cosp = 1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);
	euler.x = float(std::atan2(sinr_cosp, cosr_cosp));

	// pitch (y-axis rotation)
	double sinp = 2 * (quaternion.w * quaternion.y - quaternion.z * quaternion.x);
	if (std::abs(sinp) >= 1)
		euler.y = float(std::copysign(M_PI / 2, sinp)); // use 90 degrees if out of range
	else
		euler.y = float(std::asin(sinp));

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (quaternion.w * quaternion.z + quaternion.x * quaternion.y);
	double cosy_cosp = 1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
	euler.z = float(std::atan2(siny_cosp, cosy_cosp));

	//convert to degrees from radians from http://cplusplus.com/forum/beginner/144006/
	euler.x = (euler.x * 180.0f) / (float)M_PI;
	euler.y = (euler.y * 180.0f) / (float)M_PI;
	euler.z = (euler.z * 180.0f) / (float)M_PI;

	return euler;
}
