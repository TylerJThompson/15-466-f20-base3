#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//functions for helping with the camera rotation since I understand Euler angles way better than quaternions
	//these functions are from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	glm::quat euler_to_quaternion(glm::vec3 euler);
	glm::vec3 quaternion_to_euler(glm::quat quaternion);

	//background music:
	std::shared_ptr< Sound::PlayingSample > bgm;

	//struct for musical note collectibles
	struct Collectible {
		uint8_t index = 0;
		Scene::Transform *transform = nullptr;
		std::shared_ptr< Sound::PlayingSample > note = nullptr;
	} bflat, c, d, eflat, f, g, a;

	//used to determine where to send the collectibles after the start sequence
	std::list< Scene::Transform* > collect_locations;

	//used to know when to play notes/bgm at start
	bool in_start_sequence = true;
	bool played_start_once = false;
	float time_since_start = 0.0f;

	//transforms for the ground panels (used for collision determing height)
	std::list< Scene::Transform* > ground_transforms;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//used for jumping:
	float jump_velocity = 0.0f;

	//keeps track of which collectible the player should be trying to collect:
	int collect_index = 0;

	//keep track if the player has won:
	bool winner = false;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;
};
