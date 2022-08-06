#include "Debug.h"
#include "VoxelManager.h"

void Debug::Log(const char* input_text)
{
	std::cout << input_text << "\n";
}

void Debug::Log(std::string input_text)
{
	std::cout << input_text.c_str() << "\n";
}

void Debug::Initialize(GLFWwindow* window)
{

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	IO = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void Debug::PreRender()
{
	if (WIREFRAME_TOGGLE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Debug::Render()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	double new_time = glfwGetTime();
	double cur_time = new_time - LAST_FRAME_TIME;
	float fps_time = (float)(1 / cur_time);

	LAST_FRAME_TIME = new_time;

	ImGui::NewFrame();
	ImGui::Begin("DEBUG WINDOW");

	// VSYNC SETTING

	std::string vsync_text = "VSYNC: ON";
	if (VSYNC_TOGGLE)
		vsync_text = "VSYNC: OFF";

	if (ImGui::Button(vsync_text.c_str()))
	{
		VSYNC_TOGGLE = !VSYNC_TOGGLE;
		glfwSwapInterval(!VSYNC_TOGGLE);

	}

	// WIREFRAME 

	std::string wire_text = "WIREFRAME: OFF";
	if (WIREFRAME_TOGGLE)
		wire_text = "WIREFRAME: ON";

	if (ImGui::Button(wire_text.c_str()))
		WIREFRAME_TOGGLE = !WIREFRAME_TOGGLE;

	// FRUSTUM SETTINGS

	std::string frustum_text = "FRUSTUM CULLING: OFF";
	if (VoxelManager::VOXEL_CONFIG_FRUSTUM)
		frustum_text = "FRUSTUM CULLING: ON";

	if (ImGui::Button(frustum_text.c_str()))
	{
		VoxelManager::VOXEL_CONFIG_FRUSTUM = !VoxelManager::VOXEL_CONFIG_FRUSTUM;
	}


	std::string frustum_update_text = "FRUSTUM CULLING UPDATE: OFF";
	if (Camera::FRUSTUM_UPDATE)
		frustum_update_text = "FRUSTUM CULLING UPDATE: ON";

	if (ImGui::Button(frustum_update_text.c_str()))
	{
		Camera::FRUSTUM_UPDATE = !Camera::FRUSTUM_UPDATE;
	}

	FPS_STORAGE.push_back(fps_time);
	if (FPS_STORAGE.size() > 200)
		FPS_STORAGE.erase(FPS_STORAGE.begin());

	if (fps_time > FPS_LARGEST_FRAME)
		FPS_LARGEST_FRAME = fps_time;

	std::string chunk_triange_count = std::to_string(VoxelManager::CURRENT_TRIANGLE_COUNT);

	ImGui::Text("");
	ImGui::Text(std::string("CURRENT CHUNK TRIANGLE COUNT: " + chunk_triange_count).c_str());
	ImGui::Text("");
	ImGui::Text("CURRENT FPS:");
	ImGui::PlotHistogram("", &FPS_STORAGE[0], FPS_STORAGE.size(), 0, NULL, 0.0f, FPS_LARGEST_FRAME, ImVec2(300, 100));

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Debug::Dispose()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

ImGuiIO Debug::IO;
bool Debug::VSYNC_TOGGLE = false;

float              Debug::FPS_LARGEST_FRAME;
std::vector<float> Debug::FPS_STORAGE;
double             Debug::LAST_FRAME_TIME;
bool               Debug::WIREFRAME_TOGGLE;