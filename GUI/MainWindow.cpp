#include "MainWindow.h"
#include "imgui_stl.h"
#include "CustomWidgets.h"
#include <iostream>

MainWindow::MainWindow()
{
	processFilter = "ClassDumper3.exe";
	RefreshProcessList();
}

MainWindow::~MainWindow()
{
}

void MainWindow::Draw()
{
	ImVec2 screenSize = ImGui::GetIO().DisplaySize;

	ImVec2 windowSize(screenSize.x, screenSize.y * 0.75f);
	ImVec2 windowPos(0, 0);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	
	ImGui::Begin("ClassDumper3", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
	if (ImGui::Button("Exit"))
	{
		exit(0);
	}
	ImGui::Text("Select a process to dump classes from:");
	ImGui::Separator();
	DrawProcessList();
	ImGui::Separator();
	ImGui::SameLine();
	if (ImGui::Button("Refresh/Filter"))
	{
		RefreshProcessList();
	}
	ImGui::SameLine();
	if (ImGui::Button("Select Process"))
	{
		SelectProcess();
	}

	DrawClassList();

	ImGui::End();
}

void MainWindow::DrawProcessList()
{
	ImGui::InputText("Process filter...", &processFilter);
	if (ImGui::BeginCombo("##ProcessCombo", selectedProcessName.c_str()))
	{
		for (unsigned int n = 0; n < ProcessList.size(); n++)
		{
			bool is_selected = (selectedProcessName == ProcessList[n].name);
			if (ImGui::Selectable(ProcessList[n].name.c_str(), is_selected))
			{
				Target = std::make_shared<TargetProcess>();
				Target->Setup(ProcessList[n].pid);
				if (!Target->IsValid())
				{
					Target.reset();
					break;
				}
				selectedProcessName = ProcessList[n].name;
			}
		}
		ImGui::EndCombo();
	}
}

void MainWindow::RefreshProcessList()
{
	if(processFilter.empty()) ProcessList = GetProcessList();
	else ProcessList = GetProcessList(processFilter);
}

void MainWindow::SelectProcess()
{
	if (Target && Target->IsValid())
	{
		RTTIObserver = std::make_shared<RTTI>(Target.get(), selectedProcessName);
		RTTIObserver->ProcessRTTIAsync();
		OnProcessSelected(Target, RTTIObserver);
	}
}

void MainWindow::DrawClassList()
{
	if(!RTTIObserver) return;
	if (RTTIObserver->IsAsyncProcessing())
	{
		ImGui::Text("Processing...");
		ImGui::Text("%s", RTTIObserver->GetLoadingStage().c_str());
		ImGui::Spinner("Spinner", 10, 10, 0xFF0000FF);
		return;
	}

	if(!RTTIObserver->GetClasses().size()) return;

	ImGui::BeginChildFrame(1, ImVec2(ImGui::GetWindowWidth() , ImGui::GetWindowHeight()));
	for (std::shared_ptr<_Class> cl : RTTIObserver->GetClasses())
	{
		DrawClass(cl);
	}
	ImGui::EndChildFrame();
}

void MainWindow::DrawClass(std::shared_ptr<_Class> cl)
{
	if (cl == SelectedClass)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, {255, 0, 0, 255});
		ImGui::Text(cl->FormattedName.c_str());
		ImGui::PopStyleColor(1);
	}
	else 
	{
		ImGui::Text(cl->FormattedName.c_str());
		if(ImGui::IsItemClicked(0))
		{
			OnClassSelected(cl);
			SelectedClass = cl;
		}
	}
}
