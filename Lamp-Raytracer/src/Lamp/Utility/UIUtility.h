#pragma once

#include "Lamp/Core/Base.h"
#include "Lamp/Utility/FileSystem.h"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace Lamp
{
	class Texture2D;
	class Image2D;
}

enum class NotificationType
{
	Info,
	Warning,
	Error,
	Success
};

namespace UI
{
	static uint32_t s_contextId = 0;
	static uint32_t s_stackId = 0;

	class ScopedColor
	{
	public:
		ScopedColor(ImGuiCol_ color, const glm::vec4& newColor)
			: m_Color(color)
		{
			auto& colors = ImGui::GetStyle().Colors;
			m_OldColor = colors[color];
			colors[color] = ImVec4{ newColor.x, newColor.y, newColor.z, newColor.w };
		}

		~ScopedColor()
		{
			auto& colors = ImGui::GetStyle().Colors;
			colors[m_Color] = m_OldColor;
		}

	private:
		ImVec4 m_OldColor;
		ImGuiCol_ m_Color;
	};

	class ScopedStyleFloat
	{
	public:
		ScopedStyleFloat(ImGuiStyleVar_ var, float value)
		{
			ImGui::PushStyleVar(var, value);
		}

		~ScopedStyleFloat()
		{
			ImGui::PopStyleVar();
		}
	};

	class ScopedStyleFloat2
	{
	public:
		ScopedStyleFloat2(ImGuiStyleVar_ var, const glm::vec2& value)
		{
			ImGui::PushStyleVar(var, { value.x, value.y });
		}

		~ScopedStyleFloat2()
		{
			ImGui::PopStyleVar();
		}
	};

	ImTextureID GetTextureID(Ref<Lamp::Texture2D> texture);
	ImTextureID GetTextureID(Ref<Lamp::Image2D> texture);
	ImTextureID GetTextureID(Lamp::Texture2D* texture);

	static void ShiftCursor(float x, float y)
	{
		ImVec2 pos = { ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y };
		ImGui::SetCursorPos(pos);
	}

	static bool BeginPopup(const std::string& id = "")
	{
		if (id.empty())
		{
			return ImGui::BeginPopupContextItem();
		}

		return ImGui::BeginPopupContextItem(id.c_str());
	}

	static bool BeginPopupWindow(const std::string& id = "")
	{
		if (id.empty())
		{
			return ImGui::BeginPopupContextWindow();
		}

		return ImGui::BeginPopupContextWindow(id.c_str());
	}

	static void EndPopup()
	{
		ImGui::EndPopup();
	}

	static bool InputText(const std::string& name, std::string& text, ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_None)
	{
		if (!name.empty())
		{
			ImGui::TextUnformatted(name.c_str());
			ImGui::SameLine();
		}

		std::string id = "##" + std::to_string(s_stackId++);
		return ImGui::InputTextString(id.c_str(), &text, flags);
	}

	static bool ImageButton(const std::string& id, ImTextureID textureId, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		const ImGuiID imId = window->GetID(id.c_str());

		// Default to using texture ID as ID. User can still push string/integer prefixes.
		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : g.Style.FramePadding;
		return ImGui::ImageButtonEx(imId, textureId, size, uv0, uv1, padding, bg_col, tint_col);
	}

	static bool TreeNodeImage(Ref<Lamp::Texture2D> texture, const std::string& text, ImGuiTreeNodeFlags flags)
	{
		ScopedStyleFloat2 frame{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
		ScopedStyleFloat2 spacing{ ImGuiStyleVar_ItemSpacing, { 0.f, 0.f } };

		const ImVec2 size = ImGui::CalcTextSize(text.c_str());

		ImGui::Image(GetTextureID(texture), { size.y, size.y });
		ImGui::SameLine();

		return ImGui::TreeNodeEx(text.c_str(), flags);
	}

	static bool ImageSelectable(Ref<Lamp::Texture2D> texture, const std::string& text, bool& selected)
	{
		const ImVec2 size = ImGui::CalcTextSize(text.c_str());

		ImGui::Image(GetTextureID(texture), { size.y, size.y });
		ImGui::SameLine();

		return ImGui::Selectable(text.c_str(), &selected);
	}

	static bool ImageSelectable(Ref<Lamp::Texture2D> texture, const std::string& text)
	{
		const ImVec2 size = ImGui::CalcTextSize(text.c_str());

		ImGui::Image(GetTextureID(texture), { size.y - 2.f, size.y - 2.f });
		ImGui::SameLine();

		UI::ShiftCursor(5.f, 0.f);

		return ImGui::Selectable(text.c_str());
	}

	static void TreeNodePop()
	{
		ImGui::TreePop();
	}

	static void PushId()
	{
		int id = s_contextId++;
		ImGui::PushID(id);
		s_stackId = 0;
	}

	static void PopId()
	{
		ImGui::PopID();
		s_contextId--;
	}

	static int32_t GetId()
	{
		return s_stackId++;
	}

	static bool BeginProperties(const std::string& name = "")
	{
		return ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable);
	}

	static void EndProperties()
	{
		ImGui::EndTable();
	}

	static bool ComboProperty(const std::string& text, int& currentItem, const std::vector<std::string>& strItems, float width = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();

		if (width == 0.f)
		{
			ImGui::PushItemWidth(ImGui::GetColumnWidth());
		}
		else
		{
			ImGui::PushItemWidth(width);
		}
		std::string id = "##" + std::to_string(s_stackId++);

		std::vector<const char*> items;
		std::for_each(strItems.begin(), strItems.end(), [&](const std::string& string) { items.emplace_back(string.c_str()); });

		if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool ComboProperty(const std::string& text, int& currentItem, const std::vector<const char*>& items, float width = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();

		if (width == 0.f)
		{
			ImGui::PushItemWidth(ImGui::GetColumnWidth());
		}
		else
		{
			ImGui::PushItemWidth(width);
		}
		std::string id = "##" + std::to_string(s_stackId++);
		if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Combo(const std::string& text, int& currentItem, const std::vector<const char*>& items, float width = 100.f)
	{
		bool changed = false;

		ImGui::TextUnformatted(text.c_str());

		ImGui::SameLine();

		std::string id = "##" + std::to_string(s_stackId++);

		ImGui::SetNextItemWidth(width);
		if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
		{
			changed = true;
		}

		return changed;
	}

	static bool Combo(const std::string& text, int& currentItem, const std::vector<std::string>& strItems, float width = 100.f)
	{
		bool changed = false;

		ImGui::TextUnformatted(text.c_str());

		ImGui::SameLine();

		std::string id = "##" + std::to_string(s_stackId++);

		std::vector<const char*> items;
		std::for_each(strItems.begin(), strItems.end(), [&](const std::string& string) { items.emplace_back(string.c_str()); });

		ImGui::SetNextItemWidth(width);
		if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int32_t)items.size()))
		{
			changed = true;
		}

		return changed;
	}

	static void* DragDropTarget(const std::string& type)
	{
		void* data = nullptr;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(type.c_str()))
			{
				data = pPayload->Data;
			}

			ImGui::EndDragDropTarget();
		}

		return data;
	}

	static void* DragDropTarget(std::initializer_list<std::string> types)
	{
		void* data = nullptr;

		for (const auto& type : types)
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(type.c_str()))
				{
					data = pPayload->Data;
				}

				ImGui::EndDragDropTarget();
			}
		}

		return data;
	}

	static bool ImageSelectable(Ref<Lamp::Texture2D> texture, const std::string& text, bool selected)
	{
		ImVec2 size = ImGui::CalcTextSize(text.c_str());
		ImGui::Image(GetTextureID(texture), { size.y, size.y }, { 0, 1 }, { 1, 0 });
		ImGui::SameLine();
		return ImGui::Selectable(text.c_str(), selected, ImGuiSelectableFlags_SpanAvailWidth);
	}

	static bool PropertyAxisColor(const std::string& text, glm::vec3& value, float resetValue = 0.f)
	{
		ScopedStyleFloat2 cellPad(ImGuiStyleVar_CellPadding, { 4.f, 0.f });

		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::Text(text.c_str());

		ImGui::TableNextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
		ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

		{
			ScopedColor color{ ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.f } };
			ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.f } };
			ScopedColor colora{ ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.f } };

			std::string butId = "X##" + std::to_string(s_stackId++);
			if (ImGui::Button(butId.c_str(), buttonSize))
			{
				value.x = resetValue;
				changed = true;
			}
		}

		ImGui::SameLine();
		std::string id = "##" + std::to_string(s_stackId++);
		changed = true;

		if (ImGui::DragFloat(id.c_str(), &value.x, 0.1f))
			changed = true;

		ImGui::PopItemWidth();
		ImGui::SameLine();

		{
			ScopedColor color{ ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.f } };
			ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.f } };
			ScopedColor colora{ ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.f } };

			std::string butId = "Y##" + std::to_string(s_stackId++);
			if (ImGui::Button(butId.c_str(), buttonSize))
			{
				value.y = resetValue;
				changed = true;
			}
		}

		ImGui::SameLine();
		id = "##" + std::to_string(s_stackId++);

		if (ImGui::DragFloat(id.c_str(), &value.y, 0.1f))
			changed = true;

		ImGui::PopItemWidth();
		ImGui::SameLine();

		{
			ScopedColor color{ ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.f } };
			ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.f } };
			ScopedColor colora{ ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.f } };

			std::string butId = "Z##" + std::to_string(s_stackId++);
			if (ImGui::Button(butId.c_str(), buttonSize))
			{
				value.z = resetValue;
				changed = true;
			}
		}

		ImGui::SameLine();
		id = "##" + std::to_string(s_stackId++);
		if (ImGui::DragFloat(id.c_str(), &value.z, 0.1f))
			changed = true;

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		return true;
	}

	static bool Property(const std::string& text, bool& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);

		if (ImGui::Checkbox(id.c_str(), &value))
		{
			changed = true;
		}

		return changed;
	}

	static bool Property(const std::string& text, int32_t& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);

		if (ImGui::DragScalar(id.c_str(), ImGuiDataType_S32, (void*)&value, 1.f))
		{
			changed = true;
		}

		return changed;
	}

	static bool Property(const std::string& text, uint32_t& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, (void*)&value, 1.f))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, int16_t& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragScalar(id.c_str(), ImGuiDataType_S16, (void*)&value, 1.f))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, uint16_t& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragScalar(id.c_str(), ImGuiDataType_U16, (void*)&value, 1.f))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, int8_t& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragScalar(id.c_str(), ImGuiDataType_S8, (void*)&value, 1.f))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, uint8_t& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragScalar(id.c_str(), ImGuiDataType_U8, (void*)&value, 1.f))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, double& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragScalar(id.c_str(), ImGuiDataType_Double, (void*)&value, 1.f))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, float& value, bool useMinMax = false, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragFloat(id.c_str(), &value, 1.f, min, max))
		{
			if (value < min && useMinMax)
			{
				value = min;
			}

			if (value > max && useMinMax)
			{
				value = max;
			}

			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, glm::vec2& value, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragFloat2(id.c_str(), glm::value_ptr(value), 1.f, min, max))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, glm::vec3& value, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragFloat3(id.c_str(), glm::value_ptr(value), 1.f, min, max))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, glm::vec4& value, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragFloat4(id.c_str(), glm::value_ptr(value), 1.f, min, max))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, const std::string& value, bool readOnly = false)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (InputText("", const_cast<std::string&>(value), readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, std::string& value, bool readOnly = false)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (InputText("", value, readOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
		{
			changed = true;
		}

		return changed;
	}

	static bool PropertyColor(const std::string& text, glm::vec4& value)
	{
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value)))
		{
			return true;
		}

		return false;
	}

	static bool PropertyColor(const std::string& text, glm::vec3& value)
	{
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value)))
		{
			return true;
		}

		return false;
	}

	static bool Property(const std::string& text, std::filesystem::path& path)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string sPath = path.string();
		ImGui::PushItemWidth(ImGui::GetColumnWidth() - ImGui::CalcTextSize("Open...").x - 20.f);

		if (InputText("", sPath))
		{
			path = std::filesystem::path(sPath);
			changed = true;
		}

		ImGui::PopItemWidth();
		ImGui::SameLine();

		std::string buttonId = "Open...##" + std::to_string(s_stackId++);
		if (ImGui::Button(buttonId.c_str(), { ImGui::GetContentRegionAvail().x, 25.f }))
		{
			auto newPath = FileSystem::OpenFile("All (*.*)\0*.*\0");
			if (!newPath.empty())
			{
				path = newPath;
				changed = true;
			}
		}

		//if (auto ptr = UI::DragDropTarget("CONTENT_BROWSER_ITEM"))
		//{
		//	const wchar_t* inPath = (const wchar_t*)ptr;
		//	std::filesystem::path newPath = std::filesystem::path("assets") / inPath;

		//	path = newPath;
		//	changed = true;
		//}

		return changed;
	}
}
