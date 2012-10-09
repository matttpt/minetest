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

#include <IParticleSystemSceneNode.h>
#include <IParticleEmitter.h>
#include <string>
#include <sstream>
#include "irrlichttypes_extrabloated.h"
#include "constants.h" // for BS
#include "util/serialize.h"
#include "tile.h"
#include "light.h"
#include "particle.h"

/*
	ParticleEmitter factory mechanism
*/
ParticleEmitter * ParticleEmitter::create(u8 type, v3f pos, const ParticleEmitterDef & def,
	const std::string & extradata, scene::ISceneManager * smgr, ITextureSource * tsrc)
{
	switch(type)
	{
		case PARTICLE_EMITTER_TYPE_NONE:
			return NULL;
		case PARTICLE_EMITTER_TYPE_POINT:
			return new PointParticleEmitter(pos, def, extradata, smgr, tsrc);
		case PARTICLE_EMITTER_TYPE_BOX:
			return new BoxParticleEmitter(pos, def, extradata, smgr, tsrc);
		case PARTICLE_EMITTER_TYPE_SPHERE:
			return new SphereParticleEmitter(pos, def, extradata, smgr, tsrc);
		default:
			return NULL;
	}
}

/*
 	ParticleEmitterDef serialization and deserialization
*/
void ParticleEmitterDef::serialize(std::ostringstream & os)
{
	writeF1000(os, direction.X);
	writeF1000(os, direction.Y);
	writeF1000(os, direction.Z);
	writeU32(os, minParticlesPerSec);
	writeU32(os, maxParticlesPerSec);
	writeU32(os, minLifeMillis);
	writeU32(os, maxLifeMillis);
	writeS32(os, maxAngleDifferenceDegrees);
	writeF1000(os, minSize.Width);
	writeF1000(os, minSize.Height);
	writeF1000(os, maxSize.Width);
	writeF1000(os, maxSize.Height);
	os << serializeString(textureName);
}

void ParticleEmitterDef::deserialize(std::istringstream & is, ParticleEmitterDef & def)
{
	float x, y, z, minw, minh, maxw, maxh;
	x = readF1000(is);
	y = readF1000(is);
	z = readF1000(is);
	def.direction = core::vector3df(x, y, z);
	def.minParticlesPerSec = readU32(is);
	def.maxParticlesPerSec = readU32(is);
	def.minLifeMillis = readU32(is);
	def.maxLifeMillis = readU32(is);
	def.maxAngleDifferenceDegrees = readS32(is);
	minw = readF1000(is);
	minh = readF1000(is);
	def.minSize = core::vector2df(minw, minh);
	maxw = readF1000(is);
	maxh = readF1000(is);
	def.maxSize = core::vector2df(maxw, maxh);
	def.textureName = deSerializeString(is);
}
	

/*
	Common emitter setup helper
*/
static void common_particle_emitter_setup(v3f pos,
	scene::IParticleSystemSceneNode * node,
	const ParticleEmitterDef & def, ITextureSource * tsrc)
{
	// Set position and material data
	node->setPosition(pos);
	node->setMaterialFlag(video::EMF_LIGHTING, false);
	node->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	node->setMaterialFlag(video::EMF_FOG_ENABLE, true);
	node->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
	node->setMaterialTexture(0, tsrc->getTextureRaw(def.textureName));
	node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
}

/*
	Common emitter light update routine
*/
static void common_particle_emitter_light_update(u8 light,
	scene::IParticleEmitter * emitter)
{
	u8 light_decoded = decode_light(light);
	video::SColor color(255, light_decoded, light_decoded, light_decoded);
	emitter->setMinStartColor(color);
	emitter->setMaxStartColor(color);
}

/*
	PointParticleEmitter
*/
PointParticleEmitter::PointParticleEmitter(v3f pos, const ParticleEmitterDef & def,
	const std::string & extradata, scene::ISceneManager * smgr, ITextureSource * tsrc)
{
	// Create particle system
	m_node = smgr->addParticleSystemSceneNode(false);

	// Create and set emitter
	m_emitter = m_node->createPointEmitter(
		def.direction,
		def.minParticlesPerSec,
		def.maxParticlesPerSec,
		video::SColor(255, 255, 255, 255),
		video::SColor(255, 255, 255, 255),
		def.minLifeMillis,
		def.maxLifeMillis,
		def.maxAngleDifferenceDegrees,
		core::dimension2df(def.minSize.Width * BS, def.minSize.Height * BS),
		core::dimension2df(def.maxSize.Width * BS, def.maxSize.Height * BS));
	m_node->setEmitter(m_emitter);
	
	// Do common setup
	common_particle_emitter_setup(pos, m_node, def, tsrc);
}

void PointParticleEmitter::updateLight(u8 light)
{
	if(m_emitter)
		common_particle_emitter_light_update(light, m_emitter);
}

/*
	BoxParticleEmitter
*/
BoxParticleEmitter::BoxParticleEmitter(v3f pos, const ParticleEmitterDef & def,
	const std::string & extradata, scene::ISceneManager * smgr, ITextureSource * tsrc)
{
	// Deserialize extra data
	std::istringstream is(extradata, std::ios::binary);
	f32 minedge_x = readF1000(is) * BS;
	f32 minedge_y = readF1000(is) * BS;
	f32 minedge_z = readF1000(is) * BS;
	f32 maxedge_x = readF1000(is) * BS;
	f32 maxedge_y = readF1000(is) * BS;
	f32 maxedge_z = readF1000(is) * BS;
	core::aabbox3df box(minedge_x, minedge_y, minedge_z, maxedge_x, maxedge_y, maxedge_z);

	// Create particle system
	m_node = smgr->addParticleSystemSceneNode(false);

	// Create and set emitter
	m_emitter = m_node->createBoxEmitter(
		box,
		def.direction,
		def.minParticlesPerSec,
		def.maxParticlesPerSec,
		video::SColor(255, 255, 255, 255),
		video::SColor(255, 255, 255, 255),
		def.minLifeMillis,
		def.maxLifeMillis,
		def.maxAngleDifferenceDegrees,
		core::dimension2df(def.minSize.Width * BS, def.minSize.Height * BS),
		core::dimension2df(def.maxSize.Width * BS, def.maxSize.Height * BS));
	m_node->setEmitter(m_emitter);

	// Do common setup
	common_particle_emitter_setup(pos, m_node, def, tsrc);
}

void BoxParticleEmitter::updateLight(u8 light)
{
	if(m_emitter)
		common_particle_emitter_light_update(light, m_emitter);
}

std::string BoxParticleEmitter::serializeExtraData(const core::aabbox3df & box)
{
	std::ostringstream os(std::ios::binary);
	writeF1000(os, box.MinEdge.X);
	writeF1000(os, box.MinEdge.Y);
	writeF1000(os, box.MinEdge.Z);
	writeF1000(os, box.MaxEdge.X);
	writeF1000(os, box.MaxEdge.Y);
	writeF1000(os, box.MaxEdge.Z);
	return os.str();
}

/*
	SphereParticleEmitter
*/
SphereParticleEmitter::SphereParticleEmitter(v3f pos, const ParticleEmitterDef & def,
	const std::string & extradata, scene::ISceneManager * smgr, ITextureSource * tsrc)
{
	// Deserialize extra data
	std::istringstream is(extradata, std::ios::binary);
	f32 radius = readF1000(is) * BS;
	
	// Create particle system
	m_node = smgr->addParticleSystemSceneNode(false);

	// Create and set emitter
	m_emitter = m_node->createSphereEmitter(
		core::vector3df(0, 0, 0),
		radius,
		def.direction,
		def.minParticlesPerSec,
		def.maxParticlesPerSec,
		video::SColor(255, 255, 255, 255),
		video::SColor(255, 255, 255, 255),
		def.minLifeMillis,
		def.maxLifeMillis,
		def.maxAngleDifferenceDegrees,
		core::dimension2df(def.minSize.Width * BS, def.minSize.Height * BS),
		core::dimension2df(def.maxSize.Width * BS, def.maxSize.Height * BS));
	m_node->setEmitter(m_emitter);

	// Do common setup
	common_particle_emitter_setup(pos, m_node, def, tsrc);
}

void SphereParticleEmitter::updateLight(u8 light)
{
	if(m_emitter)
		common_particle_emitter_light_update(light, m_emitter);
}

std::string SphereParticleEmitter::serializeExtraData(f32 radius)
{
	std::ostringstream os(std::ios::binary);
	writeF1000(os, radius);
	return os.str();
}