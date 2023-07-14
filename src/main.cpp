#define GUEEPO2D_MAIN
#include <gueepo2d.h>

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 360;
static const float PI = 3.1414;

// Debug?
static gueepo::Texture* pinkTexture = nullptr;

// The Bird
static const float GRAVITY = 250.0;
static float JUMP_UP = 165.0f;
static const float MAX_ROTATION = PI/4;
static const float MIN_ROTATION = -PI/4;
static const float DEATH_Y_MIN = -180.0f;
static const float DEATH_Y_MAX = 180.0f;
static const int BIRD_ANIMATION_FRAMES_COUNT = 9;
static gueepo::Texture* bluebirdTexture;
static struct {
	gueepo::TextureRegion* AnimationFrames[BIRD_ANIMATION_FRAMES_COUNT];
	const float TimeInFrame = 0.075f;
	float TimeInCurrentFrame = 0.0f;
	int CurrentFrame = 0;

	gueepo::math::vec2 Position;
	gueepo::math::vec2 Acceleration;
	float Rotation = 0.0f;
	gueepo::math::vec2 Size;
	gueepo::math::rect CollisionRect;
} MainBird;

// Terrain
static gueepo::Texture* terrainTilemap;
static struct {
	gueepo::TextureRegion* topLeft; 
	gueepo::TextureRegion* topMid; 
	gueepo::TextureRegion* topRight;

	gueepo::TextureRegion* midLeft;
	gueepo::TextureRegion* midMid;
	gueepo::TextureRegion* midRight;

	gueepo::TextureRegion* bottomLeft;
	gueepo::TextureRegion* bottomMid; 
	gueepo::TextureRegion* bottomRight;
} improvisedTiles;

// Obstacles...
static float OBSTACLE_SPEED = 50.0f;
static const int SPIKE_BLOCK_ANIMATION_COUNT = 4;
static const int SPIKE_BLOCK_COUNT = 3;
static gueepo::Texture* spikeBlock = nullptr;
static struct {
	gueepo::TextureRegion* AnimationFrames[SPIKE_BLOCK_ANIMATION_COUNT];
	float TimeInFrame = 0.25f;
	int CurrentFrame;
	float TimeInCurrentFrame;

	gueepo::math::vec2 Position;
	gueepo::math::vec2 Size;
} SpikeBlock[SPIKE_BLOCK_COUNT];

class GLAPPY : public gueepo::Application {
public:
	GLAPPY() : Application("glappy2D ", SCREEN_WIDTH, SCREEN_HEIGHT) {}
	~GLAPPY() {}

	void Application_OnInitialize() override;
	void Application_OnInput(const gueepo::InputState& currentInputState) override;
	void Application_OnUpdate(float DeltaTime) override;
	void Application_OnDeinitialize() override;
	void Application_OnRender() override;

private:
	gueepo::FontSprite* m_kenneySquareMini = nullptr;
	gueepo::OrtographicCamera* m_camera = nullptr;
};

void GLAPPY::Application_OnInitialize() {
	m_camera = new gueepo::OrtographicCamera(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_camera->SetBackgroundColor(0.1f, 0.5f, 0.6f, 1.0f);

	pinkTexture = gueepo::Texture::Create("./assets/glappy/Pink.png");
	
	terrainTilemap = gueepo::Texture::Create("./assets/glappy/terrain.png");
	improvisedTiles.topLeft = new gueepo::TextureRegion(terrainTilemap, 96, 0, 16, 16, TOP_LEFT);
	improvisedTiles.topMid = new gueepo::TextureRegion(terrainTilemap, 96 + 16, 0, 16, 16, TOP_LEFT);
	improvisedTiles.topRight = new gueepo::TextureRegion(terrainTilemap, 96 + 32, 0, 16, 16, TOP_LEFT);

	bluebirdTexture = gueepo::Texture::Create("./assets/glappy/bluebird.png");
	for (int i = 0; i < BIRD_ANIMATION_FRAMES_COUNT; i++) {
		MainBird.AnimationFrames[i] = new gueepo::TextureRegion(bluebirdTexture, i * 32, 0, 32, 32);
	}
	MainBird.Position.x = -200.0f;
	MainBird.Size.x = -64.0f;
	MainBird.Size.y = 64.0f;

	spikeBlock = gueepo::Texture::Create("./assets/glappy/spikeblock.png");
	for (int i = 0; i < SPIKE_BLOCK_ANIMATION_COUNT; i++) {
		gueepo::TextureRegion* frame = new gueepo::TextureRegion(spikeBlock, 54 * i, 0, 54, 52, TOP_LEFT);

		for (int j = 0; j < SPIKE_BLOCK_COUNT; j++) {
			SpikeBlock[j].AnimationFrames[i] = frame;
		}
	}

	// gueepo::Font* kenneySquareMiniFontFile = gueepo::Font::CreateNewFont("./assets/Kenney Fonts/Fonts/Kenney Mini Square Mono.ttf");
	/*
	if (kenneySquareMiniFontFile != nullptr) {
		m_kenneySquareMini = new gueepo::FontSprite(kenneySquareMiniFontFile, 48);
	}
	*/
	
	// Setting up Obstacles
	float BlockPosition = 0.0f;
	for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
		SpikeBlock[i].Position.x = BlockPosition;
		SpikeBlock[i].Position.y = 0.0f;
		BlockPosition += 256.0f;

		SpikeBlock[i].Size.x = 108.0f;
		SpikeBlock[i].Size.y = 104.0f;
	}
}

void GLAPPY::Application_OnInput(const gueepo::InputState& currentInputState) {
	if (currentInputState.Mouse.WasMouseKeyPressedThisFrame(gueepo::Mousecode::MOUSE_LEFT)) {
		MainBird.Acceleration.y = JUMP_UP;
		MainBird.Rotation = MAX_ROTATION;
	}

	// DEBUG
	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_R)) {
		MainBird.Position.y = 0.0f;
	}

	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_W)) {
		JUMP_UP += 1.0f;
		LOG_INFO("JUMP UP: {0}", JUMP_UP);
	}

	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_S)) {
		JUMP_UP -= 1.0f;
		LOG_INFO("JUMP UP: {0}", JUMP_UP);
	}
}

void GLAPPY::Application_OnUpdate(float DeltaTime) {
	// Updating Main Bird Animation
	MainBird.TimeInCurrentFrame += DeltaTime;

	if (MainBird.TimeInCurrentFrame > MainBird.TimeInFrame) {
		MainBird.CurrentFrame = (MainBird.CurrentFrame + 1) % BIRD_ANIMATION_FRAMES_COUNT;
		MainBird.TimeInCurrentFrame = 0.0f;
	}

	// Applying Gravity to the Bird
	MainBird.Acceleration.y -= (GRAVITY * DeltaTime);
	MainBird.Position.y += (MainBird.Acceleration.y * DeltaTime);

	// Applying Rotation to the Bird
	if (MainBird.Acceleration.y < 0) {
		MainBird.Rotation -= DeltaTime;
	}

	if (MainBird.Rotation < MIN_ROTATION) {
		MainBird.Rotation = MIN_ROTATION;
	}

	// Updating Obstacles
	for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
		// updating animation
		SpikeBlock[i].TimeInCurrentFrame += DeltaTime;
		if (SpikeBlock[i].TimeInCurrentFrame > SpikeBlock[i].TimeInFrame) {
			SpikeBlock[i].TimeInCurrentFrame = 0.0f;
			SpikeBlock[i].CurrentFrame = (SpikeBlock[i].CurrentFrame + 1) % SPIKE_BLOCK_ANIMATION_COUNT;
		}

		SpikeBlock[i].Position.x -= OBSTACLE_SPEED * DeltaTime;
	}

	// Updating Collision
	MainBird.CollisionRect.bottomLeft.x = (MainBird.Position.x - (MainBird.Size.x / 2.0f));
	MainBird.CollisionRect.bottomLeft.y = (MainBird.Position.y - (MainBird.Size.y / 2.0f));
	MainBird.CollisionRect.topRight.x = (MainBird.Position.x + (MainBird.Size.x / 2.0f));
	MainBird.CollisionRect.topRight.y = (MainBird.Position.y + (MainBird.Size.y / 2.0f));

	if (MainBird.Position.y < DEATH_Y_MIN || MainBird.Position.y > DEATH_Y_MAX) {
		// todo: DEATH!
	}
}

void GLAPPY::Application_OnDeinitialize() {

}

void GLAPPY::Application_OnRender() {
	gueepo::Renderer::BeginFrame(*m_camera);
	gueepo::Color bgColor = m_camera->GetBackGroundColor();
	gueepo::Renderer::Clear(bgColor.rgba);

	// drawing the floor...
	int x_position = -320;
	for (int i = 0; i < 32; i++) {
		gueepo::Renderer::Draw(
			improvisedTiles.topMid,
			x_position,
			-168,
			32,
			32
		);

		x_position += 32;
	}
	
	gueepo::Renderer::Draw(MainBird.AnimationFrames[MainBird.CurrentFrame],
		static_cast<int>(MainBird.Position.x),
		static_cast<int>(MainBird.Position.y),
		static_cast<int>(MainBird.Size.x),
		static_cast<int>(MainBird.Size.y),
		MainBird.Rotation
	);

	// DEBUG. DRAWING COLLISIONS
	gueepo::Renderer::Draw(
		pinkTexture,
		static_cast<int>(MainBird.Position.x),
		static_cast<int>(MainBird.Position.y),
		static_cast<int>(MainBird.CollisionRect.GetSize().x),
		static_cast<int>(MainBird.CollisionRect.GetSize().y),
		gueepo::Color(1.0f, 1.0f, 1.0f, 0.3f)
	);

	// Rendering the Obstacles
	for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
		gueepo::Renderer::Draw(
			SpikeBlock[i].AnimationFrames[SpikeBlock[i].CurrentFrame],
			SpikeBlock[i].Position.x,
			SpikeBlock[i].Position.y,
			SpikeBlock[i].Size.x,
			SpikeBlock[i].Size.y
		);
	}

	/*
	float textWidth = m_kenneySquareMini->GetWidthOf("gueepo2D");
	gueepo::Renderer::DrawString(
		m_kenneySquareMini, 
		"gueepo2D", 
		gueepo::math::vec2(-textWidth/2.0f, -50.0f), 
		1.0f, 
		gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f)
	);
	*/
	gueepo::Renderer::EndFrame();
}

gueepo::Application* gueepo::CreateApplication(int argc, char** argv) {
	unreferenced(argc);
	unreferenced(argv);

	return new GLAPPY();
}
