#include <algorithm>
#include <cassert>

#include "SDL.h"

#include "OptionsPanel.h"

#include "CursorAlignment.h"
#include "PauseMenuPanel.h"
#include "RichTextString.h"
#include "TextAlignment.h"
#include "TextBox.h"
#include "../Entities/Player.h"
#include "../Game/Game.h"
#include "../Game/GameData.h"
#include "../Game/Options.h"
#include "../Game/PlayerInterface.h"
#include "../Math/Vector2.h"
#include "../Media/Color.h"
#include "../Media/FontManager.h"
#include "../Media/FontName.h"
#include "../Media/PaletteFile.h"
#include "../Media/PaletteName.h"
#include "../Media/TextureFile.h"
#include "../Media/TextureManager.h"
#include "../Media/TextureName.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Texture.h"
#include "../Utilities/String.h"

const std::string OptionsPanel::FPS_TEXT = "FPS Limit: ";
const std::string OptionsPanel::RESOLUTION_SCALE_TEXT = "Resolution Scale: ";
const std::string OptionsPanel::PLAYER_INTERFACE_TEXT = "Player Interface: ";
const std::string OptionsPanel::VERTICAL_FOV_TEXT = "Vertical FOV: ";
const std::string OptionsPanel::CURSOR_SCALE_TEXT = "Cursor Scale: ";
const std::string OptionsPanel::LETTERBOX_ASPECT_TEXT = "Letterbox Aspect: ";
const std::string OptionsPanel::HORIZONTAL_SENSITIVITY_TEXT = "H. Sensitivity: ";
const std::string OptionsPanel::VERTICAL_SENSITIVITY_TEXT = "V. Sensitivity: ";

OptionsPanel::OptionsPanel(Game *game)
	: Panel(game)
{
	this->titleTextBox = [game]()
	{
		const Int2 center(160, 30);

		const RichTextString richText(
			"Options",
			FontName::A,
			Color::White,
			TextAlignment::Center,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			center, richText, game->getRenderer()));
	}();

	this->backToPauseTextBox = [game]()
	{
		const Int2 center(
			Renderer::ORIGINAL_WIDTH - 30,
			Renderer::ORIGINAL_HEIGHT - 15);

		const RichTextString richText(
			"Return",
			FontName::Arena,
			Color::White,
			TextAlignment::Center,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			center, richText, game->getRenderer()));
	}();

	this->fpsTextBox = [game]()
	{
		const int x = 20;
		const int y = 45;

		const RichTextString richText(
			OptionsPanel::FPS_TEXT + std::to_string(game->getOptions().getTargetFPS()),
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->resolutionScaleTextBox = [game]()
	{
		const int x = 20;
		const int y = 65;

		const std::string text = OptionsPanel::RESOLUTION_SCALE_TEXT +
			String::fixedPrecision(game->getOptions().getResolutionScale(), 2);

		const RichTextString richText(
			text,
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->playerInterfaceTextBox = [game]()
	{
		const int x = 20;
		const int y = 85;

		const std::string text = OptionsPanel::PLAYER_INTERFACE_TEXT +
			OptionsPanel::getPlayerInterfaceString(game->getOptions().getPlayerInterface());

		const RichTextString richText(
			text,
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->verticalFOVTextBox = [game]()
	{
		const int x = 20;
		const int y = 105;

		const std::string text = OptionsPanel::VERTICAL_FOV_TEXT +
			String::fixedPrecision(game->getOptions().getVerticalFOV(), 1);

		const RichTextString richText(
			text,
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->cursorScaleTextBox = [game]()
	{
		const int x = 20;
		const int y = 125;

		const std::string text = OptionsPanel::CURSOR_SCALE_TEXT +
			String::fixedPrecision(game->getOptions().getCursorScale(), 1);

		const RichTextString richText(
			text,
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->letterboxAspectTextBox = [game]()
	{
		const int x = 20;
		const int y = 145;

		const std::string text = OptionsPanel::LETTERBOX_ASPECT_TEXT +
			String::fixedPrecision(game->getOptions().getLetterboxAspect(), 2);

		const RichTextString richText(
			text,
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->hSensitivityTextBox = [game]()
	{
		const int x = 175;
		const int y = 45;

		const std::string text = OptionsPanel::HORIZONTAL_SENSITIVITY_TEXT +
			String::fixedPrecision(game->getOptions().getHorizontalSensitivity(), 1);

		const RichTextString richText(
			text,
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->vSensitivityTextBox = [game]()
	{
		const int x = 175;
		const int y = 65;

		const std::string text = OptionsPanel::VERTICAL_SENSITIVITY_TEXT +
			String::fixedPrecision(game->getOptions().getVerticalSensitivity(), 1);

		const RichTextString richText(
			text,
			FontName::Arena,
			Color::White,
			TextAlignment::Left,
			game->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			x, y, richText, game->getRenderer()));
	}();

	this->backToPauseButton = []()
	{
		const Int2 center(
			Renderer::ORIGINAL_WIDTH - 30,
			Renderer::ORIGINAL_HEIGHT - 15);

		auto function = [](Game *game)
		{
			std::unique_ptr<Panel> pausePanel(new PauseMenuPanel(game));
			game->setPanel(std::move(pausePanel));
		};
		return std::unique_ptr<Button<Game*>>(new Button<Game*>(
			center, 40, 16, function));
	}();

	this->fpsUpButton = []()
	{
		const int x = 85;
		const int y = 41;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const int newFPS = options.getTargetFPS() + 5;
			options.setTargetFPS(newFPS);
			panel->updateFPSText(newFPS);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->fpsDownButton = [this]()
	{
		const int x = this->fpsUpButton->getX();
		const int y = this->fpsUpButton->getY() + this->fpsUpButton->getHeight();
		const int width = this->fpsUpButton->getWidth();
		const int height = this->fpsUpButton->getHeight();
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const int newFPS = std::max(options.getTargetFPS() - 5, options.MIN_FPS);
			options.setTargetFPS(newFPS);
			panel->updateFPSText(newFPS);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->resolutionScaleUpButton = []()
	{
		const int x = 120;
		const int y = 61;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options, Renderer &renderer)
		{
			const double newResolutionScale = std::min(
				options.getResolutionScale() + 0.05, options.MAX_RESOLUTION_SCALE);
			options.setResolutionScale(newResolutionScale);
			panel->updateResolutionScaleText(newResolutionScale);

			// Resize the game world rendering.
			const Int2 windowDimensions = renderer.getWindowDimensions();
			const bool fullGameWindow = options.getPlayerInterface() == PlayerInterface::Modern;
			renderer.resize(windowDimensions.x, windowDimensions.y,
				newResolutionScale, fullGameWindow);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&, Renderer&>>(
			new Button<OptionsPanel*, Options&, Renderer&>(x, y, width, height, function));
	}();

	this->resolutionScaleDownButton = [this]()
	{
		const int x = this->resolutionScaleUpButton->getX();
		const int y = this->resolutionScaleUpButton->getY() +
			this->resolutionScaleUpButton->getHeight();
		const int width = this->resolutionScaleUpButton->getWidth();
		const int height = this->resolutionScaleUpButton->getHeight();
		auto function = [](OptionsPanel *panel, Options &options, Renderer &renderer)
		{
			const double newResolutionScale = std::max(
				options.getResolutionScale() - 0.05, options.MIN_RESOLUTION_SCALE);
			options.setResolutionScale(newResolutionScale);
			panel->updateResolutionScaleText(newResolutionScale);

			// Resize the game world rendering.
			const Int2 windowDimensions = renderer.getWindowDimensions();
			const bool fullGameWindow = options.getPlayerInterface() == PlayerInterface::Modern;
			renderer.resize(windowDimensions.x, windowDimensions.y,
				newResolutionScale, fullGameWindow);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&, Renderer&>>(
			new Button<OptionsPanel*, Options&, Renderer&>(x, y, width, height, function));
	}();

	this->playerInterfaceButton = []()
	{
		const int x = 136;
		const int y = 86;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options,
			Player &player, Renderer &renderer)
		{
			// Toggle the player interface option.
			auto newPlayerInterface = (options.getPlayerInterface() == PlayerInterface::Classic) ?
				PlayerInterface::Modern : PlayerInterface::Classic;
			options.setPlayerInterface(newPlayerInterface);
			panel->updatePlayerInterfaceText(newPlayerInterface);

			// If classic mode, make sure the player is looking straight forward.
			// This is a restriction on the camera to retain the original feel.
			if (newPlayerInterface == PlayerInterface::Classic)
			{
				const Double2 groundDirection = player.getGroundDirection();
				const Double3 lookAtPoint = player.getPosition() +
					Double3(groundDirection.x, 0.0, groundDirection.y);
				player.lookAt(lookAtPoint);
			}

			// Resize the game world rendering.
			const Int2 windowDimensions = renderer.getWindowDimensions();
			const bool fullGameWindow = newPlayerInterface == PlayerInterface::Modern;
			renderer.resize(windowDimensions.x, windowDimensions.y,
				options.getResolutionScale(), fullGameWindow);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&, Player&, Renderer&>>(
			new Button<OptionsPanel*, Options&, Player&, Renderer&>(x, y, width, height, function));
	}();

	this->verticalFOVUpButton = []()
	{
		const int x = 105;
		const int y = 101;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newVerticalFOV = std::min(options.getVerticalFOV() + 5.0,
				Options::MAX_VERTICAL_FOV);
			options.setVerticalFOV(newVerticalFOV);
			panel->updateVerticalFOVText(newVerticalFOV);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->verticalFOVDownButton = [this]()
	{
		const int x = this->verticalFOVUpButton->getX();
		const int y = this->verticalFOVUpButton->getY() + this->verticalFOVUpButton->getHeight();
		const int width = this->verticalFOVUpButton->getWidth();
		const int height = this->verticalFOVUpButton->getHeight();
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newVerticalFOV = std::max(options.getVerticalFOV() - 5.0,
				Options::MIN_VERTICAL_FOV);
			options.setVerticalFOV(newVerticalFOV);
			panel->updateVerticalFOVText(newVerticalFOV);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->cursorScaleUpButton = []()
	{
		const int x = 99;
		const int y = 121;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newCursorScale = std::min(options.getCursorScale() + 0.10,
				Options::MAX_CURSOR_SCALE);
			options.setCursorScale(newCursorScale);
			panel->updateCursorScaleText(newCursorScale);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->cursorScaleDownButton = [this]()
	{
		const int x = this->cursorScaleUpButton->getX();
		const int y = this->cursorScaleUpButton->getY() + this->cursorScaleUpButton->getHeight();
		const int width = this->cursorScaleUpButton->getWidth();
		const int height = this->cursorScaleUpButton->getHeight();
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newCursorScale = std::max(options.getCursorScale() - 0.10,
				Options::MIN_CURSOR_SCALE);
			options.setCursorScale(newCursorScale);
			panel->updateCursorScaleText(newCursorScale);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->letterboxAspectUpButton = []()
	{
		const int x = 120;
		const int y = 141;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options, Renderer &renderer)
		{
			const double newLetterboxAspect = std::min(options.getLetterboxAspect() + 0.010,
				Options::MAX_LETTERBOX_ASPECT);
			options.setLetterboxAspect(newLetterboxAspect);
			panel->updateLetterboxAspectText(newLetterboxAspect);
			renderer.setLetterboxAspect(newLetterboxAspect);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&, Renderer&>>(
			new Button<OptionsPanel*, Options&, Renderer&>(x, y, width, height, function));
	}();

	this->letterboxAspectDownButton = [this]()
	{
		const int x = this->letterboxAspectUpButton->getX();
		const int y = this->letterboxAspectUpButton->getY() +
			this->letterboxAspectUpButton->getHeight();
		const int width = this->letterboxAspectUpButton->getWidth();
		const int height = this->letterboxAspectUpButton->getHeight();
		auto function = [](OptionsPanel *panel, Options &options, Renderer &renderer)
		{
			const double newLetterboxAspect = std::max(options.getLetterboxAspect() - 0.010,
				Options::MIN_LETTERBOX_ASPECT);
			options.setLetterboxAspect(newLetterboxAspect);
			panel->updateLetterboxAspectText(newLetterboxAspect);
			renderer.setLetterboxAspect(newLetterboxAspect);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&, Renderer&>>(
			new Button<OptionsPanel*, Options&, Renderer&>(x, y, width, height, function));
	}();

	this->hSensitivityUpButton = []()
	{
		const int x = 255;
		const int y = 41;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newHorizontalSensitivity = std::min(
				options.getHorizontalSensitivity() + 0.50, Options::MAX_HORIZONTAL_SENSITIVITY);
			options.setHorizontalSensitivity(newHorizontalSensitivity);
			panel->updateHorizontalSensitivityText(newHorizontalSensitivity);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->hSensitivityDownButton = [this]()
	{
		const int x = this->hSensitivityUpButton->getX();
		const int y = this->hSensitivityUpButton->getY() +
			this->hSensitivityUpButton->getHeight();
		const int width = this->hSensitivityUpButton->getWidth();
		const int height = this->hSensitivityUpButton->getHeight();
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newHorizontalSensitivity = std::max(
				options.getHorizontalSensitivity() - 0.50, Options::MIN_HORIZONTAL_SENSITIVITY);
			options.setHorizontalSensitivity(newHorizontalSensitivity);
			panel->updateHorizontalSensitivityText(newHorizontalSensitivity);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->vSensitivityUpButton = [this]()
	{
		const int x = 256;
		const int y = 61;
		const int width = 8;
		const int height = 8;
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newVerticalSensitivity = std::min(
				options.getVerticalSensitivity() + 0.50, Options::MAX_VERTICAL_SENSITIVITY);
			options.setVerticalSensitivity(newVerticalSensitivity);
			panel->updateVerticalSensitivityText(newVerticalSensitivity);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();

	this->vSensitivityDownButton = [this]()
	{
		const int x = this->vSensitivityUpButton->getX();
		const int y = this->vSensitivityUpButton->getY() +
			this->vSensitivityUpButton->getHeight();
		const int width = this->vSensitivityUpButton->getWidth();
		const int height = this->vSensitivityUpButton->getHeight();
		auto function = [](OptionsPanel *panel, Options &options)
		{
			const double newVerticalSensitivity = std::max(
				options.getVerticalSensitivity() - 0.50, Options::MIN_VERTICAL_SENSITIVITY);
			options.setVerticalSensitivity(newVerticalSensitivity);
			panel->updateVerticalSensitivityText(newVerticalSensitivity);
		};
		return std::unique_ptr<Button<OptionsPanel*, Options&>>(
			new Button<OptionsPanel*, Options&>(x, y, width, height, function));
	}();
}

OptionsPanel::~OptionsPanel()
{

}

std::string OptionsPanel::getPlayerInterfaceString(PlayerInterface playerInterface)
{
	return (playerInterface == PlayerInterface::Classic) ? "Classic" : "Modern";
}

void OptionsPanel::updateFPSText(int fps)
{
	assert(this->fpsTextBox.get() != nullptr);

	this->fpsTextBox = [this, fps]()
	{
		const RichTextString &oldRichText = this->fpsTextBox->getRichText();

		const RichTextString richText(
			OptionsPanel::FPS_TEXT + std::to_string(fps),
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->fpsTextBox->getX(), this->fpsTextBox->getY(), richText,
			this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::updateResolutionScaleText(double resolutionScale)
{
	assert(this->resolutionScaleTextBox.get() != nullptr);

	this->resolutionScaleTextBox = [this, resolutionScale]()
	{
		const RichTextString &oldRichText = this->resolutionScaleTextBox->getRichText();

		const RichTextString richText(
			OptionsPanel::RESOLUTION_SCALE_TEXT + String::fixedPrecision(resolutionScale, 2),
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->resolutionScaleTextBox->getX(), this->resolutionScaleTextBox->getY(),
			richText, this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::updatePlayerInterfaceText(PlayerInterface playerInterface)
{
	assert(this->playerInterfaceTextBox.get() != nullptr);

	this->playerInterfaceTextBox = [this, playerInterface]()
	{
		const RichTextString &oldRichText = this->playerInterfaceTextBox->getRichText();

		const std::string text = OptionsPanel::PLAYER_INTERFACE_TEXT +
			this->getPlayerInterfaceString(playerInterface);

		const RichTextString richText(
			text,
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->playerInterfaceTextBox->getX(), this->playerInterfaceTextBox->getY(),
			richText, this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::updateVerticalFOVText(double verticalFOV)
{
	assert(this->verticalFOVTextBox.get() != nullptr);

	this->verticalFOVTextBox = [this, verticalFOV]()
	{
		const RichTextString &oldRichText = this->verticalFOVTextBox->getRichText();

		const std::string text = OptionsPanel::VERTICAL_FOV_TEXT +
			String::fixedPrecision(verticalFOV, 1);

		const RichTextString richText(
			text,
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->verticalFOVTextBox->getX(), this->verticalFOVTextBox->getY(),
			richText, this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::updateCursorScaleText(double cursorScale)
{
	assert(this->cursorScaleTextBox.get() != nullptr);

	this->cursorScaleTextBox = [this, cursorScale]()
	{
		const RichTextString &oldRichText = this->cursorScaleTextBox->getRichText();

		const std::string text = OptionsPanel::CURSOR_SCALE_TEXT +
			String::fixedPrecision(cursorScale, 1);

		const RichTextString richText(
			text,
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->cursorScaleTextBox->getX(), this->cursorScaleTextBox->getY(),
			richText, this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::updateLetterboxAspectText(double letterboxAspect)
{
	assert(this->letterboxAspectTextBox.get() != nullptr);

	this->letterboxAspectTextBox = [this, letterboxAspect]()
	{
		const RichTextString &oldRichText = this->letterboxAspectTextBox->getRichText();

		const std::string text = OptionsPanel::LETTERBOX_ASPECT_TEXT +
			String::fixedPrecision(letterboxAspect, 2);

		const RichTextString richText(
			text,
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->letterboxAspectTextBox->getX(), this->letterboxAspectTextBox->getY(),
			richText, this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::updateHorizontalSensitivityText(double hSensitivity)
{
	assert(this->hSensitivityTextBox.get() != nullptr);

	this->hSensitivityTextBox = [this, hSensitivity]()
	{
		const RichTextString &oldRichText = this->hSensitivityTextBox->getRichText();

		const std::string text = OptionsPanel::HORIZONTAL_SENSITIVITY_TEXT +
			String::fixedPrecision(hSensitivity, 1);

		const RichTextString richText(
			text,
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->hSensitivityTextBox->getX(), this->hSensitivityTextBox->getY(),
			richText, this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::updateVerticalSensitivityText(double vSensitivity)
{
	assert(this->vSensitivityTextBox.get() != nullptr);

	this->vSensitivityTextBox = [this, vSensitivity]()
	{
		const RichTextString &oldRichText = this->vSensitivityTextBox->getRichText();

		const std::string text = OptionsPanel::VERTICAL_SENSITIVITY_TEXT +
			String::fixedPrecision(vSensitivity, 1);

		const RichTextString richText(
			text,
			oldRichText.getFontName(),
			oldRichText.getColor(),
			oldRichText.getAlignment(),
			this->getGame()->getFontManager());

		return std::unique_ptr<TextBox>(new TextBox(
			this->vSensitivityTextBox->getX(), this->vSensitivityTextBox->getY(),
			richText, this->getGame()->getRenderer()));
	}();
}

void OptionsPanel::drawTooltip(const std::string &text, Renderer &renderer)
{
	const Texture tooltip(Panel::createTooltip(
		text, FontName::D, this->getGame()->getFontManager(), renderer));

	const auto &inputManager = this->getGame()->getInputManager();
	const Int2 originalPosition = renderer.nativePointToOriginal(
		inputManager.getMousePosition());
	const int mouseX = originalPosition.x;
	const int mouseY = originalPosition.y;
	const int x = ((mouseX + 8 + tooltip.getWidth()) < Renderer::ORIGINAL_WIDTH) ?
		(mouseX + 8) : (mouseX - tooltip.getWidth());
	const int y = ((mouseY + tooltip.getHeight()) < Renderer::ORIGINAL_HEIGHT) ?
		mouseY : (mouseY - tooltip.getHeight());

	renderer.drawToOriginal(tooltip.get(), x, y);
}

std::pair<SDL_Texture*, CursorAlignment> OptionsPanel::getCurrentCursor() const
{
	auto &textureManager = this->getGame()->getTextureManager();
	const auto &texture = textureManager.getTexture(
		TextureFile::fromName(TextureName::SwordCursor),
		PaletteFile::fromName(PaletteName::Default));
	return std::make_pair(texture.get(), CursorAlignment::TopLeft);
}

void OptionsPanel::handleEvent(const SDL_Event &e)
{
	const auto &inputManager = this->getGame()->getInputManager();
	bool escapePressed = inputManager.keyPressed(e, SDLK_ESCAPE);

	if (escapePressed)
	{
		this->backToPauseButton->click(this->getGame());
	}

	bool leftClick = inputManager.mouseButtonPressed(e, SDL_BUTTON_LEFT);

	if (leftClick)
	{
		const Int2 mousePosition = inputManager.getMousePosition();
		const Int2 mouseOriginalPoint = this->getGame()->getRenderer()
			.nativePointToOriginal(mousePosition);

		// Check for various button clicks.
		if (this->fpsUpButton->contains(mouseOriginalPoint))
		{
			this->fpsUpButton->click(this, this->getGame()->getOptions());
		}
		else if (this->fpsDownButton->contains(mouseOriginalPoint))
		{
			this->fpsDownButton->click(this, this->getGame()->getOptions());
		}
		else if (this->resolutionScaleUpButton->contains(mouseOriginalPoint))
		{
			this->resolutionScaleUpButton->click(this, this->getGame()->getOptions(),
				this->getGame()->getRenderer());
		}
		else if (this->resolutionScaleDownButton->contains(mouseOriginalPoint))
		{
			this->resolutionScaleDownButton->click(this, this->getGame()->getOptions(),
				this->getGame()->getRenderer());
		}
		else if (this->playerInterfaceButton->contains(mouseOriginalPoint))
		{
			this->playerInterfaceButton->click(this, this->getGame()->getOptions(),
				this->getGame()->getGameData().getPlayer(), this->getGame()->getRenderer());
		}
		else if (this->verticalFOVUpButton->contains(mouseOriginalPoint))
		{
			this->verticalFOVUpButton->click(this, this->getGame()->getOptions());
		}
		else if (this->verticalFOVDownButton->contains(mouseOriginalPoint))
		{
			this->verticalFOVDownButton->click(this, this->getGame()->getOptions());
		}
		else if (this->cursorScaleUpButton->contains(mouseOriginalPoint))
		{
			this->cursorScaleUpButton->click(this, this->getGame()->getOptions());
		}
		else if (this->cursorScaleDownButton->contains(mouseOriginalPoint))
		{
			this->cursorScaleDownButton->click(this, this->getGame()->getOptions());
		}
		else if (this->letterboxAspectUpButton->contains(mouseOriginalPoint))
		{
			this->letterboxAspectUpButton->click(this, this->getGame()->getOptions(),
				this->getGame()->getRenderer());
		}
		else if (this->letterboxAspectDownButton->contains(mouseOriginalPoint))
		{
			this->letterboxAspectDownButton->click(this, this->getGame()->getOptions(),
				this->getGame()->getRenderer());
		}
		else if (this->hSensitivityUpButton->contains(mouseOriginalPoint))
		{
			this->hSensitivityUpButton->click(this, this->getGame()->getOptions());
		}
		else if (this->hSensitivityDownButton->contains(mouseOriginalPoint))
		{
			this->hSensitivityDownButton->click(this, this->getGame()->getOptions());
		}
		else if (this->vSensitivityUpButton->contains(mouseOriginalPoint))
		{
			this->vSensitivityUpButton->click(this, this->getGame()->getOptions());
		}
		else if (this->vSensitivityDownButton->contains(mouseOriginalPoint))
		{
			this->vSensitivityDownButton->click(this, this->getGame()->getOptions());
		}
		else if (this->backToPauseButton->contains(mouseOriginalPoint))
		{
			this->backToPauseButton->click(this->getGame());
		}
	}
}

void OptionsPanel::render(Renderer &renderer)
{
	// Clear full screen.
	renderer.clearNative();
	renderer.clearOriginal();

	// Set palette.
	auto &textureManager = this->getGame()->getTextureManager();
	textureManager.setPalette(PaletteFile::fromName(PaletteName::Default));

	// Draw solid background.
	renderer.clearOriginal(Color(70, 70, 78));

	// Draw buttons.
	const auto &arrows = textureManager.getTexture(
		TextureFile::fromName(TextureName::UpDown),
		PaletteFile::fromName(PaletteName::CharSheet));
	renderer.drawToOriginal(arrows.get(), this->fpsUpButton->getX(),
		this->fpsUpButton->getY());
	renderer.drawToOriginal(arrows.get(), this->resolutionScaleUpButton->getX(),
		this->resolutionScaleUpButton->getY());
	renderer.drawToOriginal(arrows.get(), this->verticalFOVUpButton->getX(),
		this->verticalFOVUpButton->getY());
	renderer.drawToOriginal(arrows.get(), this->cursorScaleUpButton->getX(),
		this->cursorScaleUpButton->getY());
	renderer.drawToOriginal(arrows.get(), this->letterboxAspectUpButton->getX(),
		this->letterboxAspectUpButton->getY());
	renderer.drawToOriginal(arrows.get(), this->hSensitivityUpButton->getX(),
		this->hSensitivityUpButton->getY());
	renderer.drawToOriginal(arrows.get(), this->vSensitivityUpButton->getX(),
		this->vSensitivityUpButton->getY());

	Texture playerInterfaceBackground(Texture::generate(Texture::PatternType::Custom1,
		this->playerInterfaceButton->getWidth(), this->playerInterfaceButton->getHeight(),
		textureManager, renderer));
	renderer.drawToOriginal(playerInterfaceBackground.get(), this->playerInterfaceButton->getX(),
		this->playerInterfaceButton->getY());

	Texture returnBackground(Texture::generate(Texture::PatternType::Custom1,
		this->backToPauseButton->getWidth(), this->backToPauseButton->getHeight(),
		textureManager, renderer));
	renderer.drawToOriginal(returnBackground.get(), this->backToPauseButton->getX(),
		this->backToPauseButton->getY());

	// Draw text.
	renderer.drawToOriginal(this->titleTextBox->getTexture(),
		this->titleTextBox->getX(), this->titleTextBox->getY());
	renderer.drawToOriginal(this->backToPauseTextBox->getTexture(),
		this->backToPauseTextBox->getX(), this->backToPauseTextBox->getY());
	renderer.drawToOriginal(this->fpsTextBox->getTexture(),
		this->fpsTextBox->getX(), this->fpsTextBox->getY());
	renderer.drawToOriginal(this->resolutionScaleTextBox->getTexture(),
		this->resolutionScaleTextBox->getX(), this->resolutionScaleTextBox->getY());
	renderer.drawToOriginal(this->playerInterfaceTextBox->getTexture(),
		this->playerInterfaceTextBox->getX(), this->playerInterfaceTextBox->getY());
	renderer.drawToOriginal(this->verticalFOVTextBox->getTexture(),
		this->verticalFOVTextBox->getX(), this->verticalFOVTextBox->getY());
	renderer.drawToOriginal(this->cursorScaleTextBox->getTexture(),
		this->cursorScaleTextBox->getX(), this->cursorScaleTextBox->getY());
	renderer.drawToOriginal(this->letterboxAspectTextBox->getTexture(),
		this->letterboxAspectTextBox->getX(), this->letterboxAspectTextBox->getY());
	renderer.drawToOriginal(this->hSensitivityTextBox->getTexture(),
		this->hSensitivityTextBox->getX(), this->hSensitivityTextBox->getY());
	renderer.drawToOriginal(this->vSensitivityTextBox->getTexture(),
		this->vSensitivityTextBox->getX(), this->vSensitivityTextBox->getY());

	const auto &inputManager = this->getGame()->getInputManager();
	const Int2 mousePosition = inputManager.getMousePosition();
	const Int2 originalPosition = renderer.nativePointToOriginal(mousePosition);

	// Draw tooltips for certain things.
	if (this->resolutionScaleTextBox->getRect().contains(originalPosition))
	{
		this->drawTooltip("Percent of the window resolution\nto use for 3D rendering.", renderer);
	}
	else if (this->playerInterfaceTextBox->getRect().contains(originalPosition))
	{
		this->drawTooltip("Modern mode uses a new minimal\ninterface with free-look.", renderer);
	}
	else if (this->letterboxAspectTextBox->getRect().contains(originalPosition))
	{
		this->drawTooltip(std::string("1.60 represents the 'unaltered' look,\n") +
			"and 1.33 represents the 'tall pixels'\n" +
			"look on a 640x480 monitor.", renderer);
	}
	else if (this->vSensitivityTextBox->getRect().contains(originalPosition))
	{
		this->drawTooltip("Only affects vertical camera look\nin modern interface mode.", renderer);
	}

	// Scale the original frame buffer onto the native one.
	renderer.drawOriginalToNative();
}
