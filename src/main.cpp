#define GUEEPO2D_MAIN
#include <gueepo2d.h>

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 360;

class Sample : public gueepo::Application {
public:
	Sample() : Application("sample ", SCREEN_WIDTH, SCREEN_HEIGHT) {}
	~Sample() {}

	void Application_OnInitialize() override;
	void Application_OnDeinitialize() override;
	void Application_OnRender() override;

private:
	gueepo::FontSprite* m_kenneySquareMini = nullptr;
	gueepo::Texture* m_kenneyRTSSample = nullptr;
	gueepo::OrtographicCamera* m_camera;
};

void Sample::Application_OnInitialize() {
	m_camera = new gueepo::OrtographicCamera(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_camera->SetBackgroundColor(0.1f, 0.6f, 0.1f, 1.0f);
	m_kenneyRTSSample = gueepo::Texture::Create("./assets/Kenney RTS/Sample.png");
	gueepo::Font* kenneySquareMiniFontFile = gueepo::Font::CreateNewFont("./assets/Kenney Fonts/Fonts/Kenney Mini Square Mono.ttf");
	if (kenneySquareMiniFontFile != nullptr) {
		m_kenneySquareMini = new gueepo::FontSprite(kenneySquareMiniFontFile, 48);
	}
}

void Sample::Application_OnDeinitialize() {

}

void Sample::Application_OnRender() {
	gueepo::Renderer::BeginFrame(*m_camera);
	gueepo::Color bgColor = m_camera->GetBackGroundColor();
	gueepo::Renderer::Clear(bgColor.rgba);
	gueepo::Renderer::Draw(m_kenneyRTSSample); // draw the texture at position (0,0) with the texture width and height

	float textWidth = m_kenneySquareMini->GetWidthOf("gueepo2D");
	gueepo::Renderer::DrawString(
		m_kenneySquareMini, 
		"gueepo2D", 
		gueepo::math::vec2(-textWidth/2.0f, -50.0f), 
		1.0f, 
		gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f)
	);
	gueepo::Renderer::EndFrame();
}

gueepo::Application* gueepo::CreateApplication() {
	return new Sample();
}
