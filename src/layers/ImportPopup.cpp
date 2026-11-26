#include "ImportPopup.h"

#include "../utils/ScopeExit.hpp"
#include <Geode/ui/Notification.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/UndoObject.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

ImportPopup* ImportPopup::create(CCPoint selectedObjectPos) {
    ImportPopup* ret = new ImportPopup();
    if (ret && ret->init(selectedObjectPos)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

// Setups the layer
bool ImportPopup::init(CCPoint selectedObjectPos) {
    if (!Popup::init(this->m_popupSize.width, this->m_popupSize.height)) return false;

    this->m_state = State::SelectJson;

    this->setID("import-popup"_spr);
    this->m_centerObjectPos = selectedObjectPos;

    this->m_parsingText = CCLabelBMFont::create("Parsing...", "bigFont.fnt");
    this->m_parsingText->setVisible(false);
    this->m_parsingText->setPosition(this->m_popupSize / 2);
    this->m_parsingText->setScale(0.6f);

    this->m_parsedView = CCNode::create();
    this->m_parsedView->setID("parsed-view");
    this->m_parsedView->setContentSize(this->m_popupSize);
    this->m_parsedView->setVisible(false);

    this->m_parsedViewMenu = CCMenu::create();
    this->m_parsedViewMenu->setID("parsed-view-menu");
    this->m_parsedViewMenu->setContentSize(this->m_popupSize);
    this->m_parsedViewMenu->setPosition({0, 0});

    this->m_countLabel = CCLabelBMFont::create("Objects: 0", "bigFont.fnt");
    this->m_countLabel->setPosition({this->m_popupSize.width / 2, this->m_popupSize.height / 2 - 65.f});
    this->m_countLabel->setID("count-label");
    this->m_countLabel->setScale(0.4);

    auto drawLabel = CCLabelBMFont::create("Scale:", "bigFont.fnt");
    drawLabel->setPosition({this->m_popupSize.width / 2 + 65.f, this->m_popupSize.height / 2 + 15.f});
    drawLabel->setID("draw-scale-label");
    drawLabel->setScale(0.5);

    auto zLayerLabel = CCLabelBMFont::create("Z-Layer\nOffset:", "bigFont.fnt");
    zLayerLabel->setPosition({this->m_popupSize.width / 2 - 65.f, this->m_popupSize.height / 2 + 15.f});
    zLayerLabel->setID("zlayer-label");
    zLayerLabel->setScale(0.325);

    this->m_fileLabel = CCLabelBMFont::create("File: ", "bigFont.fnt");
    this->m_fileLabel->setColor({0,255,0});
    this->m_fileLabel->setPosition(this->m_popupSize.width / 2, this->m_popupSize.height / 2 + 60.f);
    this->m_fileLabel->setScale(0.45);
    this->m_fileLabel->setID("file-label");

    auto importJsonSpr = ButtonSprite::create("Pick a File");
    this->m_selectBtn = CCMenuItemSpriteExtra::create(
        importJsonSpr, this, menu_selector(ImportPopup::importJSON)
    );
    this->m_selectBtn->setID("pick-btn");
    this->m_selectBtn->setPosition(this->m_selectBtn->getPosition() + this->m_popupSize / 2);

    auto changeJsonSpr = ButtonSprite::create("Pick Another");
    auto changeBtn = CCMenuItemSpriteExtra::create(
        changeJsonSpr, this, menu_selector(ImportPopup::importJSON)
    );
    changeBtn->setPosition({this->m_popupSize.width / 2, this->m_popupSize.height / 2 + 90.f});
    changeBtn->setID("pick-btn");

    auto parseSpr = ButtonSprite::create("Place");
    auto parseBtn = CCMenuItemSpriteExtra::create(
        parseSpr, this, menu_selector(ImportPopup::checkAlert)
    );
    parseBtn->setPosition({ImportPopup::m_popupSize.width / 2, ImportPopup::m_popupSize.height / 2 - 95.f});
    parseBtn->setID("place-btn");

    auto helpSpr = CCSprite::createWithSpriteFrameName("GJ_helpBtn_001.png");
    auto helpBtn = CCMenuItemSpriteExtra::create(
        helpSpr, this, menu_selector(ImportPopup::onHelp)
    );
    helpBtn->setPosition(this->m_popupSize);

    this->m_drawScaleInput = TextInput::create(50.f, "Float", "bigFont.fnt");
    this->m_drawScaleInput->setAnchorPoint({0.5f, 0.5f});
    this->m_drawScaleInput->setCommonFilter(CommonFilter::Float);
    this->m_drawScaleInput->setNumericMode(true);
    this->m_drawScaleInput->setMaxCharCount(5);
    this->m_drawScaleInput->setString("1");
    this->m_drawScaleInput->setID("draw-input");
    this->m_drawScaleInput->setPosition({this->m_popupSize.width / 2 + 65.f, this->m_popupSize.height / 2 - 15.f});
    this->m_drawScaleInput->setPlaceholderScale(0.5f);
    this->m_drawScaleInput->setFontScale(0.6f);
    this->m_drawScaleInput->setCallback([this](std::string const& value) {
        this->onDrawScaleChanged(value);
    });

    this->m_zLayerInput = TextInput::create(50.f, "Int", "bigFont.fnt");
    this->m_zLayerInput->setAnchorPoint({0.5f, 0.5f});
    this->m_zLayerInput->setCommonFilter(CommonFilter::Int);
    this->m_zLayerInput->setNumericMode(true);
    this->m_zLayerInput->setMaxCharCount(5);
    this->m_zLayerInput->setString("0");
    this->m_zLayerInput->setID("zlayer-input");
    this->m_zLayerInput->setPosition({this->m_popupSize.width / 2 - 65.f, this->m_popupSize.height / 2 - 15.f});
    this->m_zLayerInput->setPlaceholderScale(0.6f);
    this->m_zLayerInput->setFontScale(0.6f);
    this->m_zLayerInput->setCallback([this](std::string const& value) {
        this->onZLayerChanged(value);
    });

    this->m_parsedView->addChild(this->m_fileLabel);
    this->m_parsedView->addChild(this->m_countLabel);
    this->m_parsedView->addChild(drawLabel);
    this->m_parsedView->addChild(zLayerLabel);

    this->m_buttonMenu->addChild(this->m_selectBtn);
    this->m_buttonMenu->addChild(helpBtn);

    this->m_parsedViewMenu->addChild(changeBtn);
    this->m_parsedViewMenu->addChild(parseBtn);
    this->m_parsedViewMenu->addChild(this->m_drawScaleInput);
    this->m_parsedViewMenu->addChild(this->m_zLayerInput);

    this->m_mainLayer->addChild(this->m_parsingText);
    this->m_mainLayer->addChild(this->m_parsedView);
    this->m_parsedView->addChild(this->m_parsedViewMenu);
    return true;
}

void ImportPopup::parseTextAnimationStep(CCNode* node) {
    CCLabelBMFont* text = static_cast<CCLabelBMFont*>(node);

    switch (this->m_parseStep) {
        case 0:
            text->setCString("Parsing");
            break;
        case 1:
            text->setCString("Parsing.");
            break;
        case 2:
            text->setCString("Parsing..");
            break;
        case 3:
            text->setCString("Parsing...");
            break;
    }

    if (this->m_parseStep == 3) {
        this->m_parseStep = 0;
        return;
    }

    this->m_parseStep++;
}

void ImportPopup::onHelp(CCObject*) {
    geode::createQuickPopup(
        "Help", "Do you want to open the guide?",
        "No", "Yes",
        [](auto* self, bool btn2) {
            if (btn2)
                geode::utils::web::openLinkInBrowser("https://github.com/lawr0ne/Geometrize2GD/wiki/Usage-Guide");
        }
    );
}

void ImportPopup::importJSON(CCObject*) {
    // Setting file pick options
    file::FilePickOptions::Filter json_file = {
        .description = "Geometrize JSON Output",
        .files = {"*.json"}
    };
    file::FilePickOptions options = {
        .defaultPath = std::nullopt,
        .filters = {json_file}
    };

    this->setInteractive(false);

    this->m_pickHolder.spawn(
        file::pick(file::PickMode::OpenFile, options),
        [this](file::PickResult result) {
            this->onFilePicked(result);
        }
    );
}

void ImportPopup::onFilePicked(file::PickResult result) {
    auto enableBtns = ScopeExit([this]() {
        this->setInteractive(true);
    });

    // Checks does Result is empty or not
    if (result.isErr()) {
        return Notification::create(
            fmt::format("Failed to open the file. Error: {}", result.unwrapErr()),
            NotificationIcon::Error
        )->show();
    }

    auto path = result.unwrap();
    if (!path.has_value()) {
        return;
    }

    // NOTE: asp::fs::Path::filename() returns a PathView (optional), and extension() has no leading dot
    auto filename = path->filename();
    this->m_filename = filename ? filename->string() : std::string("file.json");

    auto extension = path->extension();
    if (!extension || utils::string::toLower(std::string(*extension)) != "json") {
        return Notification::create(
            "Wrong file format. It must be a .json file!",
            NotificationIcon::Error
        )->show();
    }

    enableBtns.cancel();
    this->m_selectBtn->setVisible(false);
    this->m_parsedView->setVisible(false);

    this->m_parsingText->setVisible(true);
    this->m_parsingText->runAction(
        CCRepeatForever::create(
            CCSequence::create(
                CCCallFuncN::create(this->m_parsingText, callfuncN_selector(ImportPopup::parseTextAnimationStep)),
                CCDelayTime::create(0.25f),
                nullptr
            )
        )
    )->setTag(0);

    core::json2gdo::ParseOptions parse_options {
        .positionOffset = this->m_centerObjectPos,
        .drawScale = this->m_drawScale,
        .zOrderOffset = this->m_zOrderOffset
    };

    this->m_parseHolder.spawn(
        ImportPopup::parseJSON(*path, parse_options),
        [this](std::optional<core::json2gdo::ParseResult> result) {
            this->onJSONParsed(result);
        }
    );
}

arc::Future<std::optional<core::json2gdo::ParseResult>> ImportPopup::parseJSON(asp::fs::Path path, core::json2gdo::ParseOptions options) {
    auto handle = async::runtime().spawnBlocking<Result<matjson::Value>>([path]() {
        return geode::utils::file::readJson(path);
    });
    auto jsonResult = co_await handle;

    if (jsonResult.isErr()) {
        geode::queueInMainThread([]() {
            Notification::create(
                "Failed to parse the file! It may not follow the guide.",
                NotificationIcon::Error
            )->show();
        });

        co_return std::optional<core::json2gdo::ParseResult>(std::nullopt);
    }

    auto json = jsonResult.unwrap();

    if (auto temp = json["shapes"].asArray())
        json = temp.unwrap();
    else if (auto temp = json.asArray())
       json = temp.unwrap();
    else {
        geode::queueInMainThread([]() {
            Notification::create(
                "Failed to parse the file! It may not follow the guide.",
                NotificationIcon::Error
            )->show();
        });

        co_return std::optional<core::json2gdo::ParseResult>(std::nullopt);
    }

    auto parseResult = co_await core::json2gdo::asyncParse(json, options);
    co_return std::optional(parseResult);
}

void ImportPopup::onJSONParsed(std::optional<core::json2gdo::ParseResult> result) {
    auto enableBtns = ScopeExit([this]() {
        this->setInteractive(true);
    });

    this->m_parsingText->setVisible(false);
    this->m_parsingText->stopActionByTag(0);

    if (!result.has_value()) {
        if (this->m_state == State::SelectJson) {
            this->m_selectBtn->setVisible(true);
        } else if (this->m_state == State::ParsedJson) {
            this->m_parsedView->setVisible(true);
        }

        return;
    }

    auto fileText = fmt::format("File: {}", this->m_filename);
    this->m_fileLabel->setString(fileText.c_str());
    this->m_fileLabel->limitLabelWidth(this->m_popupSize.width - 10, 0.45f, 0.2f);

    this->m_objsString = result->objects;
    this->m_objsCount = result->objectsCount;

    auto countText = fmt::format("Objects: {}", this->m_objsCount);
    this->m_countLabel->setCString(countText.c_str());
    this->m_parsedView->setVisible(true);
    this->m_state = State::ParsedJson;

    Notification::create(
        "File is parsed.",
        NotificationIcon::Success
    )->show();
}

// Checks does object count is bigger than 5k. If so, it shows a warning
void ImportPopup::checkAlert(CCObject*) {
    if (this->m_objsCount < 5000) {
        return this->place();
    }

    geode::createQuickPopup(
        "Alert",
        "This will place more than <cy>5000 objects</c>\nAre you sure?",
        "No", "Yes",
        [this](auto, bool btn2) {
            if (btn2) {
                this->place();
            }
        }
    );
}

// Places the objects inside GD Editor
void ImportPopup::place() {
    // Checking does it has parsed any objects
    if (this->m_objsString.empty()) {
        Notification::create(
            "No objects added.",
            NotificationIcon::Error
        )->show();
        return this->onClose(nullptr);
    }

    // Getting both LevelEditorLayer and EditorUI
    auto activeEditorLayer = LevelEditorLayer::get();
    auto activeEditorUI = activeEditorLayer->m_editorUI;

    // Deleting selected objects
    activeEditorUI->onDeleteSelected(nullptr);

    // Create objects from string and flip Y-axis
    auto objectsArray = activeEditorLayer->createObjectsFromString(this->m_objsString.c_str(), true, true);
    activeEditorUI->flipObjectsY(objectsArray);

    // Add to undo stack and select objects
    activeEditorLayer->m_undoObjects->addObject(UndoObject::createWithArray(objectsArray, UndoCommand::Paste));
    activeEditorUI->selectObjects(objectsArray, true);

    // Update UI and notify user
    activeEditorUI->updateButtons();
    Notification::create(
        "Successfully converted to gd objects!",
        NotificationIcon::Success
    )->show();

    // Closing the popup
    this->onClose(nullptr);
}

// Checks the value inside inputs to avoid unwanted crashes
void ImportPopup::onDrawScaleChanged(std::string const& value) {
    auto num = utils::numFromString<float>(value);
    if (num.isErr() || num.unwrap() <= 0) {
        return this->m_drawScaleInput->setTextColor(ccColor4B{255, 0, 0, 255});
    }

    this->m_drawScaleInput->setTextColor(ccColor4B{255, 255, 255, 255});
    this->m_drawScale = num.unwrap();
}

void ImportPopup::onZLayerChanged(std::string const& value) {
    auto num = utils::numFromString<int>(value);
    if (num.isErr()) {
        return this->m_zLayerInput->setTextColor(ccColor4B{255, 0, 0, 255});
    }

    this->m_zLayerInput->setTextColor(ccColor4B{255, 255, 255, 255});
    this->m_zOrderOffset = num.unwrap();
}

// Enables/disables every interactive element and the ability to close the popup.
// In Geode v6 the close button is no longer a child of m_buttonMenu, so it has to be handled manually.
void ImportPopup::setInteractive(bool enabled) {
    this->m_buttonMenu->setEnabled(enabled);
    this->m_parsedViewMenu->setEnabled(enabled);
    if (this->m_closeBtn) {
        this->m_closeBtn->setEnabled(enabled);
    }
    this->m_allowedExit = enabled;
}

void ImportPopup::onClose(CCObject* sender) {
    if (this->m_allowedExit) {
        Popup::onClose(sender);
    }
}

void ImportPopup::keyBackClicked() {
    this->onClose(nullptr);
}
