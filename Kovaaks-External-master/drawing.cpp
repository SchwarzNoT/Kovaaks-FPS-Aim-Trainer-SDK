#include "Overlay.h"

bool boxESP;
bool bRounded;
float rounding;

bool lineESP;
bool pieceESP;
bool playerNameESP;
bool healthESP;
bool quishaESP;
bool bAimbot;
int aimKey = 0;
char keyName[128] = "Aim Key";
static const char* aimModes[]{ "CrossHair", "Distance" };
int  aimPriority = 0;
float smoothX = 1.f;
float smoothY = 1.f;
bool bKillChanger;
int killCount;
bool bDamageDone;
float damageDone;


ImFont* Font;



void overlay::menuDrawing()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImFont* Font = ImGui::GetFont();
	ImGui::SetNextWindowSize(ImVec2(600, 400));


	ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoDecoration);
	{


		ImGui::Checkbox("Box", &boxESP);

		if (boxESP) {

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);
			ImGui::Checkbox("Rounded", &bRounded);
			if (bRounded) {
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);

				ImGui::SliderFloat("Box Rounding", &rounding, 0.f, 15.f);
			}


		}


		ImGui::Checkbox("Line", &lineESP);

		ImGui::Checkbox("Quisha", &quishaESP);

		ImGui::Checkbox("Aimbot", &bAimbot);

		if (bAimbot) {

			ImGui::SameLine(0, 20.f);
			if (ImGui::Button(keyName, { 80, 20 })) {

				aimKey = util::setAimKey();
				SecureZeroMemory(keyName, sizeof(keyName));
				GetKeyNameTextA(MapVirtualKey(aimKey, 0) << 16, keyName, sizeof(keyName));

			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);

			ImGui::Combo("Aim Priority", &aimPriority, aimModes, IM_ARRAYSIZE(aimModes));

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);

			ImGui::SliderFloat("Smooth X", &smoothX, 1.f, 30.f);

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);

			ImGui::SliderFloat("Smooth Y", &smoothY, 1.f, 30.f);


		}


		ImGui::Checkbox("Kill Count Changer", &bKillChanger);

		if (bKillChanger) {


			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);

			ImGui::InputInt("Kills", &killCount, 1.f);

		}

		ImGui::Checkbox("Damage Changer", &bDamageDone);

		if (bDamageDone) {


			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);


			ImGui::InputFloat("Damage", &damageDone, 1.f);

		}


	
		ImGui::SetCursorPos({ ImGui::GetWindowSize().x - 305.f, 5.f });
		

		ImGui::Image((void*)vtLogoImg, { 348, 61 });




	}
	ImGui::End();
}


void overlay::drawText(Vector3 World2Screen, float yOffset, const char* text)
{
	Font = ImGui::GetFont();

	ImVec2 textSize = ImGui::CalcTextSize(text);

	Font->Scale = 1.5f;

	ImGui::PushFont(Font);

	ImGui::GetBackgroundDrawList()->AddText({ (World2Screen.x - (textSize.x / 2) * (Font->Scale)) - 1.f, World2Screen.y + yOffset - textSize.y }, ImColor{ 0.f, 0.f, 0.f, 1.f }, text);
	ImGui::GetBackgroundDrawList()->AddText({ (World2Screen.x - (textSize.x / 2) * (Font->Scale)) + 1.f, World2Screen.y + yOffset - textSize.y }, ImColor{ 0.f, 0.f, 0.f, 1.f }, text);
	ImGui::GetBackgroundDrawList()->AddText({ (World2Screen.x - (textSize.x / 2) * (Font->Scale)) - 1.f, (World2Screen.y + yOffset - textSize.y) - 1.f }, ImColor{ 0.f, 0.f, 0.f, 1.f }, text);
	ImGui::GetBackgroundDrawList()->AddText({ (World2Screen.x - (textSize.x / 2) * (Font->Scale)) + 1.f, (World2Screen.y + yOffset - textSize.y) + 1.f }, ImColor{ 0.f, 0.f, 0.f, 1.f }, text);
	ImGui::GetBackgroundDrawList()->AddText({ World2Screen.x - (textSize.x / 2) * (Font->Scale) , World2Screen.y + yOffset - textSize.y }, ImColor{ 1.f, 1.f, 1.f, 1.f }, text);

	Font->Scale = 1.f;

	ImGui::PopFont();



}

void overlay::drawBox(Vector3 World2Screen) 
{


	
	if (World2Screen.x <= 4000.f && World2Screen.x >= -2000.f) {

		if (World2Screen.y <= 4000.f && World2Screen.y>= -2000.f) {

			ImGui::GetBackgroundDrawList()->AddRect({ World2Screen.x - boxDimensions.x - 1.75f , World2Screen.y - boxDimensions.y - 1.75f }, { World2Screen.x + boxDimensions.x + 1.75f, World2Screen.y + boxDimensions.y + 1.75f }, ImColor(0.f, 0.f, 0.f, 1.f), 0.f, 0, 1.5f);
			ImGui::GetBackgroundDrawList()->AddRect({ World2Screen.x - boxDimensions.x , World2Screen.y-boxDimensions.y  }, { World2Screen.x + boxDimensions.x , World2Screen.y + boxDimensions.y}, ImColor(1.f, 0.f, 0.f), 0.f, 0, 2.f);
			ImGui::GetBackgroundDrawList()->AddRect({ World2Screen.x - boxDimensions.x + 1.75f , World2Screen.y - boxDimensions.y + 1.75f }, { World2Screen.x +  boxDimensions.x - 1.75f, World2Screen.y + boxDimensions.y - 1.75f }, ImColor(0.f, 0.f, 0.f, 1.f), 0.f, 0, 1.5f);

		}


	}

}
void overlay::drawBox(Vector3 World2Screen, float rounding)
{



	if (World2Screen.x <= 4000.f && World2Screen.x >= -2000.f) {

		if (World2Screen.y <= 4000.f && World2Screen.y >= -2000.f) {

			ImGui::GetBackgroundDrawList()->AddRect({ World2Screen.x - boxDimensions.x - 1.75f , World2Screen.y - boxDimensions.y - 1.75f }, { World2Screen.x + boxDimensions.x + 1.75f, World2Screen.y + boxDimensions.y + 1.75f }, ImColor(0.f, 0.f, 0.f, 1.f), rounding, 0, 1.5f);
			ImGui::GetBackgroundDrawList()->AddRect({ World2Screen.x - boxDimensions.x , World2Screen.y - boxDimensions.y }, { World2Screen.x + boxDimensions.x , World2Screen.y + boxDimensions.y }, ImColor(1.f, 0.f, 0.f), rounding, 0, 2.f);
			ImGui::GetBackgroundDrawList()->AddRect({ World2Screen.x - boxDimensions.x + 1.75f , World2Screen.y - boxDimensions.y + 1.75f }, { World2Screen.x + boxDimensions.x - 1.75f, World2Screen.y + boxDimensions.y - 1.75f }, ImColor(0.f, 0.f, 0.f, 1.f), rounding, 0, 1.5f);

		}


	}

}


void overlay::drawLine(Vector3 World2Screen) 
{
	ImGui::GetBackgroundDrawList()->AddLine({ winProperties.width / 2 + winProperties.x, winProperties.height / 2  + winProperties.y}, { World2Screen.x, World2Screen.y }, ImColor{ 1.f, 0.f, 0.f }, 2.f);
}


void overlay::drawHealth(Vector3 World2Screen, float health)
{
	if (health > 0.f && health <= 100.f) {

		ImColor color;

		if (health == 100.f) {

			color = ImColor(0.f, 1.f, 0.f, 1.f);

		}
		else if (health >= 75.f) {

			color = ImColor(1.f, 1.f, 0.f, 1.f);

		}
		else if (health >= 50.f) {

			color = ImColor(1.f, 0.5f, 0.f, 1.f);

		}
		else if (health >= 25.f) {

			color = ImColor(1.f, 0.f, 0.f, 1.f);

		}
		else if (health > 0.f) {

			color = ImColor(.5f, 0.2f, 0.f, 1.f);

		}

		ImGui::GetBackgroundDrawList()->AddLine({ World2Screen.x - boxDimensions.x - 5.f, (World2Screen.y + boxDimensions.y) - (2 * boxDimensions.y * (health / 100.f))}, { World2Screen.x - boxDimensions.x - 5.f, World2Screen.y + boxDimensions.y  }, ImColor(0.f, 0.f, 0.f, 1.f), 5.f);

		ImGui::GetBackgroundDrawList()->AddLine({ World2Screen.x - boxDimensions.x - 5.f, (World2Screen.y + boxDimensions.y) - (2 * boxDimensions.y * (health / 100.f)) +1.f }, { World2Screen.x - boxDimensions.x - 5.f, World2Screen.y + boxDimensions.y -1.f }, color, 3.f);
	}
}





