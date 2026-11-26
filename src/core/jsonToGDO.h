#pragma once

#include <Geode/binding/GameObject.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <arc/future/Future.hpp>
#include <matjson.hpp>

#include <algorithm>
#include <cmath>
#include <set>
#include <string>

namespace core::json2gdo {
    namespace {
        constexpr int m_circleID = 497;
    }
    const std::set<int> m_validObjTypes = {5, 3, 4, 32, 8, 16};

    struct ParseOptions {
        cocos2d::CCPoint positionOffset;
        float drawScale;
        int zOrderOffset;
    };
    struct ParseResult {
        std::string objects;
        int objectsCount;
    };

    ParseResult parse(const matjson::Value &json, ParseOptions options);
    arc::Future<ParseResult> asyncParse(const matjson::Value &json, ParseOptions options);
    void rgbToHsv(float fR, float fG, float fB, float &fH, float &fS, float &fV);
}
