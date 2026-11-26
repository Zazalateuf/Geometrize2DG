#include "../layers/ImportPopup.h"

#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;

// This hook adds a custom button
class $modify(MyEditorUI, EditorUI) {

	// Checks does there are selected objects. If so, creates ImportPopup
	void onImgToGD(CCObject*) {
	    auto selectedObjs = CCArrayExt<GameObject*>(this->getSelectedObjects());

		if (selectedObjs.size() != 1) {
			return FLAlertLayer::create(
    			"Info",
                "You have to choose <cg>exactly one</c> object to be the down-left corner of the image!",
    			"OK"
			)->show();
		}

		// show() adds the popup to the scene with the standard animation (respects the Fast Menu option)
		auto importPopup = ImportPopup::create(selectedObjs[0]->getPosition());
		if (importPopup) {
			importPopup->show();
		}
	}

	CCMenuItemSpriteExtra* createImportButton() {
		return this->getSpriteButton("geometrize2gd.png"_spr, menu_selector(MyEditorUI::onImgToGD), nullptr, 0.9f);
	}

	// Since GD 2.2091 the move menu has two modes:
	//  - D-pad mode: no EditButtonBar is created (m_editButtonBar is null), the edit buttons
	//    are given to createDpadMoveMenu instead.
	//  - Classic mode: the buttons are shown in m_editButtonBar.
#ifndef GEODE_IS_IOS
	// D-pad mode (this function has no iOS binding, and iOS doesn't use the D-pad)
	void createDpadMoveMenu(CCArray* buttons) {
		if (buttons) {
			if (auto btn = this->createImportButton()) {
				buttons->addObject(btn);
			}
		}

		EditorUI::createDpadMoveMenu(buttons);
	}
#endif

	void createMoveMenu() {
		// Calling the original function
		EditorUI::createMoveMenu();

		// Classic mode only. In D-pad mode the button was already added in createDpadMoveMenu.
		if (m_dpadMoveNode || !m_editButtonBar || !m_editButtonBar->m_buttonArray) {
			return;
		}

		// Creating custom button and adding it to button array
		if (auto btn = this->createImportButton()) {
			m_editButtonBar->m_buttonArray->addObject(btn);
		}

		// Checking settings and updating buttons
		auto rows = GameManager::sharedState()->getIntGameVariable("0049");
		auto columns = GameManager::sharedState()->getIntGameVariable("0050");
		m_editButtonBar->reloadItems(rows, columns);
	}
};
