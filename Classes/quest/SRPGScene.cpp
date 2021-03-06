//
//  SRPGScene.cpp
//  Cocos2dxSRPGQuest
//
//  Created by kyokomi on 2013/10/05.
//
//

#include "SRPGScene.h"
#include "SRPGMapLayer.h"

USING_NS_CC;

SRPGScene::SRPGScene()
:m_touched(false)
{
}

SRPGScene::~SRPGScene()
{
}

Scene* SRPGScene::scene()
{
    Scene *scene = Scene::create();
    SRPGScene *layer = SRPGScene::create();
    scene->addChild(layer);
    return scene;
}

bool SRPGScene::init()
{
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    Size winSize = Director::getInstance()->getWinSize();

    // TouchEvent settings
    auto listener = EventListenerTouchOneByOne::create();
    //listener->setSwallowTouches(true);
    
    listener->onTouchBegan = CC_CALLBACK_2(SRPGScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(SRPGScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(SRPGScene::onTouchEnded, this);
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    auto* srpgMapLayer = SRPGMapLayer::create();
    srpgMapLayer->setTag(SRPGScene::kSRPGMapLayerTag);
    this->addChild(srpgMapLayer);
    
    // ----------------------------------------
    Point baseMenuPoint = Point(0, 24);
    
    // グリッド表示/非表示
    auto* menuItem1 = MenuItemLabel::create(LabelTTF::create("A", "", 24), [this](Object *pSender) {
        //ログ出力
        auto* pMapLayer = (SRPGMapLayer*) this->getChildByTag(SRPGScene::kSRPGMapLayerTag);
        if (pMapLayer)
        {
            if (pMapLayer->isShowGrid())
            	pMapLayer->hideGrid();
            else
            	pMapLayer->showGrid();
        }
    });
    menuItem1->setColor(Color3B::RED);
    menuItem1->setPosition(menuItem1->getContentSize().width * 0.5, menuItem1->getContentSize().height * 0.5);

    auto* menuItem2 = MenuItemLabel::create(LabelTTF::create("B", "", 24), [this](Object *pSender) {
        //ログ出力
        CCLOG("menuItem2が押された！");
        auto* pMapLayer = (SRPGMapLayer*) this->getChildByTag(SRPGScene::kSRPGMapLayerTag);
        if (pMapLayer) pMapLayer->clearAllMapCursor();
    });
    menuItem2->setColor(Color3B::RED);
    menuItem2->setPosition(menuItem2->getContentSize().width * 0.5, menuItem1->getContentSize().height * 0.5 + menuItem1->getPositionY() + menuItem2->getContentSize().height * 0.5);

    auto* menuItem3 = MenuItemLabel::create(LabelTTF::create("C", "", 24), [this](Object *pSender) {
        //ログ出力
        CCLOG("menuItem3が押された！");
        auto* pMapLayer = (SRPGMapLayer*) this->getChildByTag(SRPGScene::kSRPGMapLayerTag);
        if (pMapLayer)
        {
            if (pMapLayer->isMapCursor(MapDataType::MOVE_DIST))
            	pMapLayer->hideMapCursor(MapDataType::MOVE_DIST);
            else
            	pMapLayer->showMapCursor(MapDataType::MOVE_DIST);
        }
    });
    menuItem3->setColor(Color3B::RED);
    menuItem3->setPosition(menuItem3->getContentSize().width * 0.5, menuItem2->getContentSize().height * 0.5 + menuItem2->getPositionY() + menuItem3->getContentSize().height * 0.5);
    
    auto* menu = Menu::create(menuItem1, menuItem2, menuItem3, NULL);
    menu->setPosition(baseMenuPoint);
    this->addChild(menu);
    // ----------------------------------------
    
    return true;
}

void SRPGScene::onEnterTransitionDidFinish()
{
    
}

bool SRPGScene::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
    // 基点となるタップ位置を記録しておく
    m_pStartPoint = this->convertToWorldSpace(this->convertTouchToNodeSpace(touch));
	m_pDelta = Point::ZERO;
	m_touched = true;

    // updateメソッドを毎フレーム実行
	this->unscheduleUpdate();
    this->scheduleUpdate();
    
    auto* pMapLayer = (SRPGMapLayer*) this->getChildByTag(SRPGScene::kSRPGMapLayerTag);
    if (pMapLayer)
    {
        pMapLayer->onTouchBegan(touch, event);
    }
    
    return true;
}

void SRPGScene::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
    Point touchPoint = this->convertToWorldSpace(this->convertTouchToNodeSpace(touch));
    // タップを移動させた位置を記憶する
    m_pDelta = m_pStartPoint - touchPoint;
    CCLOG("m_pDelta = [%f, %f]", m_pDelta.x, m_pDelta.y);
}


void SRPGScene::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event)
{
    CCLOG("onTouchEnded = [%f, %f]", m_pDelta.x, m_pDelta.y);
    
    // タップ終了の0.3秒後くらいにupdateメソッドの毎フレーム実行をキャンセル
	if (this->isScheduled(schedule_selector(SRPGScene::touchUnSchedule)))
	{
		this->unschedule(schedule_selector(SRPGScene::touchUnSchedule));
	}
	this->scheduleOnce(schedule_selector(SRPGScene::touchUnSchedule), 0.3f);
	m_touched = false;

    // フリック対象に場合はグリッドのイベントは処理しない
    if (checkFlick()) return;
    
    auto* pMapLayer = (SRPGMapLayer*) this->getChildByTag(SRPGScene::kSRPGMapLayerTag);
    if (pMapLayer)
    {
        pMapLayer->onTouchEnded(touch, event);
    }
}

void SRPGScene::touchUnSchedule(float time)
{
	if (!m_touched) this->unscheduleUpdate();
}

void SRPGScene::onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *event)
{
    this->unscheduleUpdate();
}

void SRPGScene::update(float delta)
{
    // フリック対象でない場合はマップ移動を行わない
    if (!checkFlick()) return;
    
    auto* pMapLayer = (SRPGMapLayer*) this->getChildByTag(SRPGScene::kSRPGMapLayerTag);
    if (pMapLayer)
    {
        Point movePoint = pMapLayer->createTouchMoveMapPoint(pMapLayer->getPosition(), delta, m_pDelta);
        pMapLayer->setPosition(movePoint);
    }
}

bool SRPGScene::checkFlick()
{
    if (abs(m_pDelta.x) >= 30 || abs(m_pDelta.y) >= 30)
    {
        return true;
    }
    return false;
}


