/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2016 Davorin Učakar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <client/ExplosionImago.hh>

#include <client/Context.hh>

namespace oz
{
namespace client
{

int ExplosionImago::modelId;

Pool<ExplosionImago> ExplosionImago::pool(64);

Imago* ExplosionImago::create(const Object* obj)
{
  ExplosionImago* imago = new ExplosionImago(obj);

  modelId = obj->clazz->imagoModel;

  imago->model     = context.requestModel(modelId);
  imago->startTime = timer.time;

  return imago;
}

ExplosionImago::~ExplosionImago()
{
  context.releaseModel(modelId);
}

void ExplosionImago::draw(const Imago*)
{
  if (!model->isLoaded()) {
    return;
  }

  float time   = (timer.time - startTime).t();
  float radius = 4.0f * time * obj->dim.z;
  float alpha  = 1.0f - 2.0f * time;

  tf.model = Mat4::translation(obj->p - Point::ORIGIN);
  tf.model.scale(Vec3(radius, radius, radius));

  tf.colour.w.w = alpha*alpha;

  model->schedule(0, Model::SCENE_QUEUE);

  tf.colour.w.w = 1.0f;
}

}
}
