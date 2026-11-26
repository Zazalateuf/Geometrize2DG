#include "../layers/ImportPopup.h"

#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/GameManager.hpp>

#include <algorithm>
#include <cfloat>

using namespace geode::prelude;

namespace {
	// Computes (in world space) the bounding box that contains every menu item below `node`
	void collectItemBounds(CCNode* node, CCPoint& lo, CCPoint& hi, bool& found) {
		if (!node) return;

		if (auto item = typeinfo_cast<CCMenuItem*>(node)) {
			if (auto parent = item->getParent()) {
				auto box = item->boundingBox();
				auto a = parent->convertToWorldSpace({box.getMinX(), box.getMinY()});
				auto b = parent->convertToWorldSpace({box.getMaxX(), box.getMaxY()});
				lo.x = std::min({lo.x, a.x, b.x});
				lo.y = std::min({lo.y, a.y, b.y});
				hi.x = std::max({hi.x, a.x, b.x});
				hi.y = std::max({hi.y, a.y, b.y});
				found = true;
			}
		}

		if (auto children = node->getChildren()) {
			for (auto child : CCArrayExt<CCNode*>(children)) {
				collectItemBounds(child, lo, hi, found);
			}
		}
	}
}

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

	// Since GD 2.2091 the edit tab has two modes:
	//  - D-pad mode (default): there is no EditButtonBar (m_editButtonBar is null) and the buttons live in m_dpadMoveNode
	//  - Legacy mode: the buttons are in m_editButtonBar
	// The D-pad layout is controlled by the game (and by Node IDs), so instead of inserting the button inside it,
	// the button is placed in its own menu, right above the D-pad bar. Being a child of m_dpadMoveNode, it
	// is shown/hidden together with the edit tab.
	void addDpadImportButton() {
		auto const menuID = "import-menu"_spr;
		if (auto old = m_dpadMoveNode->getChildByID(menuID)) {
			old->removeFromParent();
		}

		auto btn = this->createImportButton();
		if (!btn) {
			log::error("Failed to create the import button");
			return;
		}

		auto winSize = CCDirector::get()->getWinSize();

		CCPoint lo {FLT_MAX, FLT_MAX};
		CCPoint hi {-FLT_MAX, -FLT_MAX};
		bool found = false;
		collectItemBounds(m_dpadMoveNode, lo, hi, found);

		CCPoint world = found
			? CCPoint {hi.x - 22.f, hi.y + 24.f}
			: CCPoint {winSize.width - 30.f, 114.f};
		world.x = std::clamp(world.x, 22.f, winSize.width - 22.f);
		world.y = std::min(world.y, winSize.height - 22.f);

		auto local = m_dpadMoveNode->convertToNodeSpace(world);
		auto nodeScale = m_dpadMoveNode->getScale();
		btn->setScale(0.8f / (nodeScale > 0.f ? nodeScale : 1.f));
		btn->setPosition(local);
		btn->setID("import-button"_spr);

		auto menu = CCMenu::create();
		menu->setID(menuID);
		menu->setContentSize({0.f, 0.f});
		menu->setPosition({0.f, 0.f});
		menu->setZOrder(100);
		menu->addChild(btn);
		m_dpadMoveNode->addChild(menu);

		log::info(
			"D-pad mode: import button placed at world ({}, {}), items found: {}, bar bounds ({}, {}) - ({}, {})",
			world.x, world.y, found, lo.x, lo.y, hi.x, hi.y
		);
	}

	void createMoveMenu() {
		// Calling the original function
		EditorUI::createMoveMenu();

		if (m_dpadMoveNode) {
			return this->addDpadImportButton();
		}

		// Legacy mode only
		if (!m_editButtonBar || !m_editButtonBar->m_buttonArray) {
			log::warn("Neither the D-pad nor the edit button bar exist, the import button was not added");
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
		log::info("Legacy mode: import button added to the edit button bar");
	}
};
