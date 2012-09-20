/*
Minetest-c55
Copyright (C) 2010-2012 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2012 matttpt (Matthew I) <matttpt@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef PARTICLE_HEADER
#define PARTICLE_HEADER

#include <string>
#include <sstream>
#include "irrlichttypes_extrabloated.h"
#include <IParticleSystemSceneNode.h>
#include <IParticleEmitter.h>

#define PARTICLE_EMITTER_TYPE_NONE   0
#define PARTICLE_EMITTER_TYPE_POINT  1
#define PARTICLE_EMITTER_TYPE_BOX    2
#define PARTICLE_EMITTER_TYPE_SPHERE 3

class ITextureSource;
class ParticleEmitter;

// Data used to construct particle emitters
struct ParticleEmitterDef
{
	core::vector3df    direction;                  // particle direction
	u32                minParticlesPerSec;         // minimum particles emitted per second
	u32                maxParticlesPerSec;         // maximum partices emitted per second
	u32                minLifeMillis;              // minimum particle lifetime in milliseconds
	u32                maxLifeMillis;              // maximum particle lifetime in milliseconds
	s32                maxAngleDifferenceDegrees;  // maximum difference in degress from a particles
	                                               // actual direction and the given direction
	core::dimension2df minSize;                    // minimum particle start size
	core::dimension2df maxSize;                    // maximum particle start size
	std::string        textureName;                // texture for particles

	void serialize(std::ostringstream & os);
	static void deserialize(std::istringstream & is, ParticleEmitterDef & def);
};

// Base particle emitter
class ParticleEmitter
{
public:
	static ParticleEmitter * create(u8 type, v3f pos,
		const ParticleEmitterDef & def, const std::string & extradata,
		scene::ISceneManager * smgr, ITextureSource * tsrc);
	virtual u8 getType() { return PARTICLE_EMITTER_TYPE_NONE; }
	virtual void suspend() = 0;
	virtual void resume() = 0;
	virtual void updateLight(u8 light) { }
	virtual ~ParticleEmitter() { }
};

// Particle emitter that emits points from a single point
class PointParticleEmitter : public ParticleEmitter
{
protected:
	scene::IParticleSystemSceneNode * m_node;
	scene::IParticleEmitter         * m_emitter;
public:
	PointParticleEmitter(v3f pos, const ParticleEmitterDef & def, const std::string & extradata, scene::ISceneManager * smgr, ITextureSource * tsrc);
	virtual u8 getType() { return PARTICLE_EMITTER_TYPE_POINT; }
	void suspend() { m_node->setEmitter(0); }
	void resume() { m_node->setEmitter(m_emitter); }
	void updateLight(u8 light);
	virtual ~PointParticleEmitter() { m_emitter->drop(); m_node->remove(); }
};

// Particle emitter that emits points from a box-shaped space
class BoxParticleEmitter : public ParticleEmitter
{
protected:
	scene::IParticleSystemSceneNode * m_node;
	scene::IParticleEmitter         * m_emitter;
public:
	BoxParticleEmitter(v3f pos, const ParticleEmitterDef & def, const std::string & extradata, scene::ISceneManager * smgr, ITextureSource * tsrc);
	virtual u8 getType() { return PARTICLE_EMITTER_TYPE_BOX; }
	void suspend() { m_node->setEmitter(0); }
	void resume() { m_node->setEmitter(m_emitter); }
	void updateLight(u8 light);
	std::string serializeExtraData(const core::aabbox3df & box);
	virtual ~BoxParticleEmitter() { m_emitter->drop(); m_node->remove(); }
};

// Particle emitter that emits points from a sphere-shaped space
class SphereParticleEmitter : public ParticleEmitter
{
protected:
	scene::IParticleSystemSceneNode * m_node;
	scene::IParticleEmitter         * m_emitter;
public:
	SphereParticleEmitter(v3f pos, const ParticleEmitterDef & def, const std::string & extradata, scene::ISceneManager * smgr, ITextureSource * tsrc);
	virtual u8 getType() { return PARTICLE_EMITTER_TYPE_SPHERE; }
	void suspend() { m_node->setEmitter(0); }
	void resume() { m_node->setEmitter(m_emitter); }
	void updateLight(u8 light);
	std::string serializeExtraData(f32 radius);
	virtual ~SphereParticleEmitter() { m_emitter->drop(); m_node->remove(); }
};

#endif // PARTICLE_HEADER