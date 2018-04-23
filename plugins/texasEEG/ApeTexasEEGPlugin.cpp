#include <iostream>
#include <math.h> 
#include "ApeTexasEEGPlugin.h"

Ape::ApeTexasEEGPlugin::ApeTexasEEGPlugin()
{
	LOG_FUNC_ENTER();
	mpKeyboard = NULL;
	mpMouse = NULL;
	mpEventManager = Ape::IEventManager::getSingletonPtr();
	mpEventManager->connectEvent(Ape::Event::Group::NODE, std::bind(&ApeTexasEEGPlugin::eventCallBack, this, std::placeholders::_1));
	mpScene = Ape::IScene::getSingletonPtr();
	mpMainWindow = Ape::IMainWindow::getSingletonPtr();
	mUserNode = Ape::NodeWeakPtr();
	mpSystemConfig = Ape::ISystemConfig::getSingletonPtr();
	mKeyCodeMap = std::map<OIS::KeyCode, bool>();
	mTranslateSpeedFactor = 1;
	mRotateSpeedFactor = 1;
	mIsSwim = false;
	mMouseMoveRelativeX = 0;
	mMouseMoveRelativeY = 0;
	mIsMouseMoved = false;
	LOG_FUNC_LEAVE();
}

Ape::ApeTexasEEGPlugin::~ApeTexasEEGPlugin()
{
	LOG_FUNC_ENTER();
	LOG_FUNC_LEAVE();
}

void Ape::ApeTexasEEGPlugin::eventCallBack(const Ape::Event& event)
{

}

void Ape::ApeTexasEEGPlugin::Init()
{
	LOG_FUNC_ENTER();

	if (auto userNode = mpScene->getNode(mpSystemConfig->getSceneSessionConfig().generatedUniqueUserNodeName).lock())
		mUserNode = userNode;

	LOG(LOG_TYPE_DEBUG, "waiting for main window");
	while (mpMainWindow->getHandle() == nullptr)
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	LOG(LOG_TYPE_DEBUG, "main window was found");
	std::stringstream hwndStrStream;
	hwndStrStream << mpMainWindow->getHandle();
	std::stringstream windowHndStr;
	windowHndStr << std::stoul(hwndStrStream.str(), nullptr, 16);
	OIS::ParamList pl;
	pl.insert(std::make_pair("WINDOW", windowHndStr.str()));
#ifdef WIN32
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#endif
	OIS::InputManager* inputManager = OIS::InputManager::createInputSystem(pl);

	if (inputManager->getNumberOfDevices(OIS::OISKeyboard) > 0)
	{
		OIS::Keyboard* keyboard = static_cast<OIS::Keyboard*>(inputManager->createInputObject(OIS::OISKeyboard, true));
		mpKeyboard = keyboard;
		mpKeyboard->setEventCallback(this);
	}
	if (inputManager->getNumberOfDevices(OIS::OISMouse) > 0)
	{
		OIS::Mouse* mouse = static_cast<OIS::Mouse*>(inputManager->createInputObject(OIS::OISMouse, true));
		mpMouse = mouse;
		mpMouse->setEventCallback(this);
		const OIS::MouseState &ms = mouse->getMouseState();
		ms.width = mpMainWindow->getWidth();
		ms.height = mpMainWindow->getHeight();
	}
	if (auto bubblesNode = mpScene->createNode("bubblesNode").lock())
	{
		bubblesNode->setPosition(Ape::Vector3(0, 0, 0));
		bubblesNode->setScale(Ape::Vector3(0.1, 0.1, 0.1));
		if (auto bubblesMeshFile = std::static_pointer_cast<Ape::IFileGeometry>(mpScene->createEntity("bubbles.mesh", Ape::Entity::GEOMETRY_FILE).lock()))
		{
			bubblesMeshFile->setFileName("bubbles.mesh");
			bubblesMeshFile->setParentNode(bubblesNode);
		}
	}
	LOG_FUNC_LEAVE();
}

void Ape::ApeTexasEEGPlugin::moveUserNodeByKeyBoard()
{
	auto userNode = mUserNode.lock();
	if (userNode)
	{
		if (mKeyCodeMap[OIS::KeyCode::KC_PGUP])
			userNode->translate(Ape::Vector3(0, 1 * mTranslateSpeedFactor, 0), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_PGDOWN])
			userNode->translate(Ape::Vector3(0, -1 * mTranslateSpeedFactor, 0), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_D])
			userNode->translate(Ape::Vector3(1 * mTranslateSpeedFactor, 0, 0), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_A])
			userNode->translate(Ape::Vector3(-1 * mTranslateSpeedFactor, 0, 0), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_W])
			userNode->translate(Ape::Vector3(0, 0, -1 * mTranslateSpeedFactor), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_S])
			userNode->translate(Ape::Vector3(0, 0, 1 * mTranslateSpeedFactor), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_LEFT])
			userNode->rotate(0.017f * mRotateSpeedFactor, Ape::Vector3(0, 1, 0), Ape::Node::TransformationSpace::WORLD);
		if (mKeyCodeMap[OIS::KeyCode::KC_RIGHT])
			userNode->rotate(-0.017f * mRotateSpeedFactor, Ape::Vector3(0, 1, 0), Ape::Node::TransformationSpace::WORLD);
		if (mKeyCodeMap[OIS::KeyCode::KC_UP])
			userNode->rotate(0.017f * mRotateSpeedFactor, Ape::Vector3(1, 0, 0), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_DOWN])
			userNode->rotate(-0.017f * mRotateSpeedFactor, Ape::Vector3(1, 0, 0), Ape::Node::TransformationSpace::LOCAL);
		if (mKeyCodeMap[OIS::KeyCode::KC_NUMPAD4])
			userNode->rotate(0.017f * mRotateSpeedFactor, Ape::Vector3(0, 0, 1), Ape::Node::TransformationSpace::WORLD);
		if (mKeyCodeMap[OIS::KeyCode::KC_NUMPAD6])
			userNode->rotate(-0.017f * mRotateSpeedFactor, Ape::Vector3(0, 0, 1), Ape::Node::TransformationSpace::WORLD);
	}
}

void Ape::ApeTexasEEGPlugin::moveUserNodeByMouse()
{
	if (auto userNode = mUserNode.lock())
	{
			if (mIsSwim)
				userNode->translate(Ape::Vector3(0, 0, -1 * mTranslateSpeedFactor), Ape::Node::TransformationSpace::LOCAL);
			if (mIsMouseMoved)
			{
				userNode->rotate(Ape::Degree(-mMouseMoveRelativeY).toRadian() * mRotateSpeedFactor, Ape::Vector3(1, 0, 0), Ape::Node::TransformationSpace::LOCAL);
				userNode->rotate(Ape::Degree(-mMouseMoveRelativeX).toRadian() * mRotateSpeedFactor, Ape::Vector3(0, 1, 0), Ape::Node::TransformationSpace::WORLD);
				mIsMouseMoved = false;
			}
	}
}

void Ape::ApeTexasEEGPlugin::Run()
{
	LOG_FUNC_ENTER();
	while (true)
	{
		if (mpKeyboard)
			mpKeyboard->capture();
		if (mpMouse)
			mpMouse->capture();
		moveUserNodeByKeyBoard();
		moveUserNodeByMouse();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	mpEventManager->disconnectEvent(Ape::Event::Group::NODE, std::bind(&ApeTexasEEGPlugin::eventCallBack, this, std::placeholders::_1));
	LOG_FUNC_LEAVE();
}

void Ape::ApeTexasEEGPlugin::Step()
{

}

void Ape::ApeTexasEEGPlugin::Stop()
{

}

void Ape::ApeTexasEEGPlugin::Suspend()
{

}

void Ape::ApeTexasEEGPlugin::Restart()
{

}

bool Ape::ApeTexasEEGPlugin::keyPressed(const OIS::KeyEvent & e)
{
	mKeyCodeMap[e.key] = true;
	return true;
}

bool Ape::ApeTexasEEGPlugin::keyReleased(const OIS::KeyEvent & e)
{
	mKeyCodeMap[e.key] = false;
	return true;
}

bool Ape::ApeTexasEEGPlugin::mouseMoved(const OIS::MouseEvent & e)
{
	/*mMouseMoveRelativeX = 0;
	if (e.state.X.rel > 0)
		mMouseMoveRelativeX = 1;
	else if (e.state.X.rel < 0)
		mMouseMoveRelativeX = -1;
	mMouseMoveRelativeY = 0;
	if (e.state.Y.rel > 0)
		mMouseMoveRelativeY = 1;
	else if (e.state.Y.rel < 0)
		mMouseMoveRelativeY = -1;*/
	mMouseMoveRelativeX = e.state.X.rel;
	mMouseMoveRelativeY = e.state.Y.rel;
	mIsMouseMoved = true;
	//LOG(LOG_TYPE_DEBUG, e.state.X.rel << "," << e.state.Y.rel);
	return true;
}

bool Ape::ApeTexasEEGPlugin::mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id)
{
	mIsSwim = true;
	return true;
}

bool Ape::ApeTexasEEGPlugin::mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id)
{
	mIsSwim = false;
	return true;
}
