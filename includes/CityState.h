#ifndef CITY_STATE_H
#define CITY_STATE_H

#include <iostream>

#include <NDK/Application.hpp>
#include <NDK/Entity.hpp>
#include <NDK/State.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/World.hpp>
#include <NDK/Components.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Utility.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>

#include "Config\TilesConfig.hpp"
#include "World/WorldMap.hpp"
#include "UserTools.hpp"
#include "SpriteLibrary.hpp"

class CityState : public Ndk::State {

public:
	CityState(Ndk::World& world, Nz::RenderWindow& window);
	void Enter(Ndk::StateMachine& fsm) override;
	void Leave(Ndk::StateMachine& fsm) override;
	bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

	void mouseLeftPressed(Nz::Vector2ui mousePosition);
	void mouseRightPressed(Nz::Vector2ui mousePosition);
	void mouseWheelMoved(float delta);
	void keyPressed(const Nz::WindowEvent::KeyEvent& k);

	

private:
	SpriteLibrary m_spriteLib;
	Ndk::World& m_world;
	const Nz::Vector2f m_windowSize;

	UserTools m_currentTool;

	WorldMap m_worldMap;
};

#endif // !CITY_STATE_H
