#define GUEEPO2D_MAIN
#include <gueepo2d.h>

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 360;
static const float PI = 3.1414;

// Game Settings
static const float OUT_OF_SCREEN_X = -375.0f;
static const float START_OF_SCREEN_X = 375.0f;
static const float TIME_TO_INCREASE_DIFFICULTY = 5.0f;
static const float DIFFICULTY_INCREASE_RATE = 1.1f;
static const float JUMP_UP_DIFFICULTY_INCREASE = 1.02f;
static const float STARTER_GRAVITY = 250.0;
static const float STARTER_JUMP_UP = 165.0f;
static const float STARTER_OBSTACLE_SPEED = 75.0f;
static float TimeElapsed = 0.0f;
static float NextIncreaseTime = TIME_TO_INCREASE_DIFFICULTY;

// Game Flow
enum class GameState {
	INTRO,
	GAMEPLAY,
	DEAD
};
static GameState CurrentState = GameState::INTRO;

// Debug
static gueepo::Texture* pinkTexture = nullptr;
static bool IsDebugEnabled = false;

// Background
static const float MAX_BACKGROUND_OFFSET = 32.0f;
static const float BACKGROUND_SPEED = 5.0f;
static struct {
	gueepo::Texture* backgroundTexture;
	gueepo::math::vec2 Position;
	int Width;
	int Height;

	float CurrentOffset = 0.0f;
	int Direction = 1;
} Background;

// The Bird
static float CurrentGravity = STARTER_GRAVITY;
static float CurrentJumpUp = STARTER_JUMP_UP;

static const float MAX_ROTATION = PI/6;
static const float MIN_ROTATION = -PI/4;
static const float DEATH_Y_MIN = -180.0f;
static const float DEATH_Y_MAX = 180.0f;
static const int BIRD_ANIMATION_FRAMES_COUNT = 9;
static gueepo::Texture* bluebirdTexture;
static const float STARTER_FRAME_TIME = 0.075f;
static const float MIN_FRAME_TIME = 0.025f;
static struct {
	gueepo::TextureRegion* AnimationFrames[BIRD_ANIMATION_FRAMES_COUNT];
	float TimeInFrame = 0.075f;
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
static int PlayerScore = 0;
static float CurrentObstacleSpeed = STARTER_OBSTACLE_SPEED;
static const float SPIKE_BLOCK_SPACING = 225.0f;
static const int SPIKE_BLOCK_ANIMATION_COUNT = 4;
static const int SPIKE_BLOCK_COUNT = 4;
static const float SPIKE_BLOCK_MAX_Y = 120;
static const float SPIKE_BLOCK_MIN_Y = -120;
static gueepo::Texture* spikeBlock = nullptr;
static struct {
	gueepo::TextureRegion* AnimationFrames[SPIKE_BLOCK_ANIMATION_COUNT];
	float TimeInFrame = 0.25f;
	int CurrentFrame;
	float TimeInCurrentFrame;

	gueepo::math::vec2 Position;
	gueepo::math::vec2 Size;
	gueepo::math::rect CollisionRect;

	gueepo::math::rect ScoreRect;
	bool IsScoreRegionEnabled;
} SpikeBlock[SPIKE_BLOCK_COUNT];

static const float GetRandomSpikeBlockY() {
	float RandomValue = gueepo::rand::Float();
	float YPosition = SPIKE_BLOCK_MIN_Y + (RandomValue * (SPIKE_BLOCK_MAX_Y - SPIKE_BLOCK_MIN_Y));
	return YPosition;
}

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

static void InitializeBird() {
	MainBird.Position.y = 0.0f;
	MainBird.Acceleration.y = 0.0f;
	MainBird.Rotation = 0.0f;
	MainBird.TimeInFrame = STARTER_FRAME_TIME;
}

static void InitializeObstacles() {
	float BlockPosition = START_OF_SCREEN_X;
	for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
		SpikeBlock[i].Position.x = BlockPosition;
		SpikeBlock[i].Position.y = GetRandomSpikeBlockY();
		BlockPosition += SPIKE_BLOCK_SPACING;

		SpikeBlock[i].Size.x = 108.0f;
		SpikeBlock[i].Size.y = 104.0f;
		SpikeBlock[i].IsScoreRegionEnabled = true;
	}
}

static void RestartGame() {
	InitializeBird();
	InitializeObstacles();

	CurrentObstacleSpeed = STARTER_OBSTACLE_SPEED;
	CurrentGravity = STARTER_GRAVITY;
	CurrentJumpUp = STARTER_JUMP_UP;
	PlayerScore = 0;
}

void GLAPPY::Application_OnInitialize() {
	gueepo::rand::Init();

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

	// Loading up Font
	gueepo::Font* kenneySquareMiniFontFile = gueepo::Font::CreateNewFont("./assets/Kenney Fonts/Fonts/Kenney Mini Square Mono.ttf");
	if (kenneySquareMiniFontFile != nullptr) {
		m_kenneySquareMini = new gueepo::FontSprite(kenneySquareMiniFontFile, 48);
		m_kenneySquareMini->SetLineGap(-24.0f);
	}

	// Setting up Background
	Background.backgroundTexture = gueepo::Texture::Create("./assets/glappy/Green.png");
	Background.Width = 12;
	Background.Height = 12;
	Background.Position.x = -300.0f;
	Background.Position.y = -150.0f;

	RestartGame();
}

void GLAPPY::Application_OnInput(const gueepo::InputState& currentInputState) {

	switch (CurrentState) {
	case GameState::INTRO: {
		if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_E)) {
			RestartGame();
			CurrentState = GameState::GAMEPLAY;
			MainBird.Acceleration.y = CurrentJumpUp;
			MainBird.Rotation = MAX_ROTATION;
		}
	} break;
	case GameState::GAMEPLAY: {
		if (currentInputState.Mouse.WasMouseKeyPressedThisFrame(gueepo::Mousecode::MOUSE_LEFT)) {
			MainBird.Acceleration.y = CurrentJumpUp;
			MainBird.Rotation = MAX_ROTATION;
		}
	} break;
	case GameState::DEAD: {
		if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_E)) {
			RestartGame();
			CurrentState = GameState::GAMEPLAY;
			MainBird.Acceleration.y = CurrentJumpUp;
			MainBird.Rotation = MAX_ROTATION;
		}
	} break;
	}
	

	// DEBUG
	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_D)) {
		IsDebugEnabled = !IsDebugEnabled;
	}

	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_R)) {
		RestartGame();
	}
}
static void UpdateGameplay(float DeltaTime);
void GLAPPY::Application_OnUpdate(float DeltaTime) {
	// Background
	Background.CurrentOffset += DeltaTime * Background.Direction * BACKGROUND_SPEED;
	if (Background.Direction < 0 && Background.CurrentOffset < 0) {
		Background.Direction = 1;
	}
	else if (Background.Direction > 0 && Background.CurrentOffset > MAX_BACKGROUND_OFFSET) {
		Background.Direction = -1;
	}

	// Updating Main Bird Animation
	MainBird.TimeInCurrentFrame += DeltaTime;

	if (MainBird.TimeInCurrentFrame > MainBird.TimeInFrame) {
		MainBird.CurrentFrame = (MainBird.CurrentFrame + 1) % BIRD_ANIMATION_FRAMES_COUNT;
		MainBird.TimeInCurrentFrame = 0.0f;
	}

	switch (CurrentState) {
	case GameState::INTRO: {

	} break;
	case GameState::GAMEPLAY: {
		TimeElapsed += DeltaTime;
		UpdateGameplay(DeltaTime);
	} break;
	case GameState::DEAD: {
		MainBird.Position.y -= DeltaTime;
		MainBird.Rotation += DeltaTime;
	} break;
	}
}

void UpdateGameplay(float DeltaTime) {
	// Applying Gravity to the Bird
	MainBird.Acceleration.y -= (CurrentGravity * DeltaTime);
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

		// moving
		SpikeBlock[i].Position.x -= CurrentObstacleSpeed * DeltaTime;
	}

	// Updating Collision
	MainBird.CollisionRect.bottomLeft.x =
		(MainBird.Position.x - (gueepo::math::abs(MainBird.Size.x) / 3.0f));
	MainBird.CollisionRect.bottomLeft.y =
		(MainBird.Position.y - (gueepo::math::abs(MainBird.Size.y) / 3.0f));
	MainBird.CollisionRect.topRight.x =
		(MainBird.Position.x + (gueepo::math::abs(MainBird.Size.x) / 3.0f));
	MainBird.CollisionRect.topRight.y =
		(MainBird.Position.y + (gueepo::math::abs(MainBird.Size.y) / 3.0f));

	for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
		SpikeBlock[i].CollisionRect.bottomLeft.x =
			(SpikeBlock[i].Position.x - (gueepo::math::abs(SpikeBlock[i].Size.x) / 3.0f));
		SpikeBlock[i].CollisionRect.bottomLeft.y =
			(SpikeBlock[i].Position.y - (gueepo::math::abs(SpikeBlock[i].Size.y) / 3.0f));
		SpikeBlock[i].CollisionRect.topRight.x =
			(SpikeBlock[i].Position.x + (gueepo::math::abs(SpikeBlock[i].Size.x) / 3.0f));
		SpikeBlock[i].CollisionRect.topRight.y =
			(SpikeBlock[i].Position.y + (gueepo::math::abs(SpikeBlock[i].Size.y) / 3.0f));

		SpikeBlock[i].ScoreRect.bottomLeft.x = SpikeBlock[i].Position.x - 5.0f;
		SpikeBlock[i].ScoreRect.topRight.x = SpikeBlock[i].Position.x + 5.0f;

		SpikeBlock[i].ScoreRect.bottomLeft.y = SpikeBlock[i].Position.y - 350.0f;
		SpikeBlock[i].ScoreRect.topRight.y = SpikeBlock[i].Position.y + 350.0f;
	}

	// Checking for Collisions
	bool Collided = false;
	for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
		if (MainBird.CollisionRect.Intersect(SpikeBlock[i].CollisionRect)) {
			Collided = true;
			break;
		}

		if (
			SpikeBlock[i].IsScoreRegionEnabled &&
			MainBird.CollisionRect.Intersect(SpikeBlock[i].ScoreRect)) {
			PlayerScore++;
			SpikeBlock[i].IsScoreRegionEnabled = false;
		}
	}

	if (Collided || MainBird.Position.y < DEATH_Y_MIN || MainBird.Position.y > DEATH_Y_MAX) {
		CurrentState = GameState::DEAD;
	}

	// Relocating Blocks
	for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
		if (SpikeBlock[i].Position.x < OUT_OF_SCREEN_X) {
			SpikeBlock[i].Position.x += SPIKE_BLOCK_SPACING * SPIKE_BLOCK_COUNT;
			SpikeBlock[i].Position.y = GetRandomSpikeBlockY();

			SpikeBlock[i].IsScoreRegionEnabled = true;
		}
	}

	// Processing Game Difficulty
	if (TimeElapsed > NextIncreaseTime) {
		NextIncreaseTime += TIME_TO_INCREASE_DIFFICULTY;

		CurrentObstacleSpeed = CurrentObstacleSpeed * DIFFICULTY_INCREASE_RATE;
		CurrentJumpUp = CurrentJumpUp * JUMP_UP_DIFFICULTY_INCREASE;
		CurrentGravity = CurrentGravity * DIFFICULTY_INCREASE_RATE;

		MainBird.TimeInFrame = (MainBird.TimeInFrame / DIFFICULTY_INCREASE_RATE);
		if (MainBird.TimeInFrame < MIN_FRAME_TIME) {
			MainBird.TimeInFrame = MIN_FRAME_TIME;
		}
	}
}

void GLAPPY::Application_OnDeinitialize() {
	// todo (lol)
}

void GLAPPY::Application_OnRender() {
	gueepo::Renderer::BeginFrame(*m_camera);
	gueepo::Color bgColor = m_camera->GetBackGroundColor();
	gueepo::Renderer::Clear(bgColor.rgba);

	// Drawing the Background
	float BackgroundX = Background.Position.x;
	float BackgroundY = Background.Position.y;
	for (int i = 0; i < Background.Height; i++) {
		BackgroundX = Background.Position.x;
		for (int j = 0; j < Background.Width; j++) {
			gueepo::Renderer::Draw(
				Background.backgroundTexture,
				BackgroundX,
				BackgroundY + static_cast<int>(Background.CurrentOffset)
			);
			BackgroundX += 64.0f;
		}
		BackgroundY += 64.0f;
	}

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

	// Drawing the Bird!!
	gueepo::Renderer::Draw(MainBird.AnimationFrames[MainBird.CurrentFrame],
		static_cast<int>(MainBird.Position.x),
		static_cast<int>(MainBird.Position.y),
		static_cast<int>(MainBird.Size.x),
		static_cast<int>(MainBird.Size.y),
		MainBird.Rotation
	);

	// Drawing Specific Stuff
	switch (CurrentState) {
	case GameState::INTRO: {
		gueepo::Renderer::DrawString(m_kenneySquareMini, "glappy2D", gueepo::math::vec2(-100.0f, 25.0f), 1.0f, gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f));
		gueepo::Renderer::DrawString(m_kenneySquareMini, "press 'e'\nto start", gueepo::math::vec2(-100.0f, -25.0f), 0.75f, gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f));

	} break;
	case GameState::GAMEPLAY: {
		// Rendering the Score
		std::string ScoreString = std::to_string(PlayerScore);
		float textWidth = m_kenneySquareMini->GetWidthOf(ScoreString.c_str());
		gueepo::Renderer::DrawString(
			m_kenneySquareMini,
			ScoreString.c_str(),
			gueepo::math::vec2(-textWidth / 2.0f, 140.0f),
			1.0f,
			gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f)
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
	} break;
	case GameState::DEAD: {
		gueepo::Renderer::DrawString(m_kenneySquareMini, "you died :(", gueepo::math::vec2(-100.0f, 25.0f), 1.0f, gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f));
		gueepo::Renderer::DrawString(m_kenneySquareMini, "press 'e'\nto restart", gueepo::math::vec2(-100.0f, -25.0f), 0.75f, gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f));
	} break;
	}

	// DEBUG. DRAWING COLLISIONS
	if (IsDebugEnabled) {
		gueepo::Renderer::Draw(
			pinkTexture,
			static_cast<int>(MainBird.Position.x),
			static_cast<int>(MainBird.Position.y),
			static_cast<int>(MainBird.CollisionRect.GetSize().x),
			static_cast<int>(MainBird.CollisionRect.GetSize().y),
			gueepo::Color(0.0f, 0.0f, 1.0f, 0.5f)
		);

		gueepo::Renderer::Draw(
			pinkTexture,
			MainBird.CollisionRect.bottomLeft.x,
			MainBird.CollisionRect.bottomLeft.y,
			5,
			5,
			gueepo::Color(0.1f, 1.0f, 0.1f, 0.5f)
		);

		gueepo::Renderer::Draw(
			pinkTexture,
			MainBird.CollisionRect.topRight.x,
			MainBird.CollisionRect.topRight.y,
			5,
			5,
			gueepo::Color(0.1f, 1.0f, 0.1f, 0.5f)
		);

		for (int i = 0; i < SPIKE_BLOCK_COUNT; i++) {
			gueepo::Renderer::Draw(
				pinkTexture,
				static_cast<int>(SpikeBlock[i].Position.x),
				static_cast<int>(SpikeBlock[i].Position.y),
				static_cast<int>(SpikeBlock[i].ScoreRect.GetSize().x),
				static_cast<int>(SpikeBlock[i].ScoreRect.GetSize().y),
				gueepo::Color(0.0f, 0.0f, 1.0f, 0.5f)
			);

			gueepo::Renderer::Draw(
				pinkTexture,
				static_cast<int>(SpikeBlock[i].Position.x),
				static_cast<int>(SpikeBlock[i].Position.y),
				static_cast<int>(SpikeBlock[i].CollisionRect.GetSize().x),
				static_cast<int>(SpikeBlock[i].CollisionRect.GetSize().y),
				gueepo::Color(1.0f, 0.1f, 0.1f, 0.5f)
			);
		}
	}

	gueepo::Renderer::EndFrame();
}

gueepo::Application* gueepo::CreateApplication(int argc, char** argv) {
	unreferenced(argc);
	unreferenced(argv);

	return new GLAPPY();
}
