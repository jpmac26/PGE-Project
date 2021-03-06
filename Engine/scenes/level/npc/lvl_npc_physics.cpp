/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2017 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../lvl_npc.h"
#include "../lvl_player.h"
#include "../lvl_block.h"
#include "../lvl_physenv.h"
#include "../lvl_bgo.h"
#include "../../scene_level.h"

void LVL_Npc::processContacts()
{
    /* *********************** Check all collided sides ***************************** */
    bool doHurt = false;
    int  hurtDamage = 0;
    bool doKill = false;
    int  killReason = DAMAGE_NOREASON;

    for(ObjectCollidersIt it = l_contactAny.begin(); it != l_contactAny.end(); it++)
    {
        PGE_Phys_Object *cEL = *it;
        switch(cEL->type)
        {
        case PGE_Phys_Object::LVLBGO:
        {
            LVL_Bgo *bgo = static_cast<LVL_Bgo *>(cEL);
            SDL_assert(bgo);
            l_pushBgo(bgo);
            break;
        }
        case PGE_Phys_Object::LVLPhysEnv:
        {
            LVL_PhysEnv *env = static_cast<LVL_PhysEnv *>(cEL);
            SDL_assert(env);
            environments_map[intptr_t(env)] = env->env_type;
            break;
        }
        case PGE_Phys_Object::LVLBlock:
        {
            LVL_Block *blk = static_cast<LVL_Block *>(cEL);
            SDL_assert(blk);
            if(blk->m_isHidden)
                break;

            l_pushBlk(blk);

            if(blk->setup->setup.lava)
            {
                doKill = true;
                killReason = DAMAGE_LAVABURN;
            }

            if(m_onSlopeFloorTopAlign && !m_slopeFloor.has && m_stand &&
               (blk->m_momentum.velY <= m_momentum.velY))
            {
                switch(blk->m_shape)
                {
                case PGE_physBody::SL_LeftBottom:
                {
                    if((m_momentum.left() <= blk->m_momentum.left()) &&
                       (m_momentum.left() + m_momentum.velX > m_momentum.left()))
                    {
                        double k = blk->m_momentum.h / blk->m_momentum.w;
                        double newx = m_momentum.x + m_momentum.velX;
                        double newy = blk->m_momentum.y + ((newx - blk->m_momentum.x) * k) - m_momentum.h;
                        m_momentum.velY = fabs(m_momentum.y - newy);
                        m_slopeFloor.has = true;
                        m_slopeFloor.shape = blk->m_shape;
                        m_slopeFloor.rect  = blk->m_momentum.rect();
                        m_onSlopeYAdd = 0.0;
                    }
                    break;
                }
                case PGE_physBody::SL_RightBottom:
                {
                    if((m_momentum.right() >= blk->m_momentum.right()) &&
                       (m_momentum.right() + m_momentum.velX < m_momentum.right()))
                    {
                        double k = blk->m_momentum.h / blk->m_momentum.w;
                        double newx = m_momentum.x + m_momentum.velX;
                        double newy = blk->m_momentum.y + ((blk->m_momentum.right() - newx - m_momentum.w) * k) - m_momentum.h;
                        m_momentum.velY = fabs(m_momentum.y - newy);
                        m_slopeFloor.has = true;
                        m_slopeFloor.shape = blk->m_shape;
                        m_slopeFloor.rect  = blk->m_momentum.rect();
                        m_onSlopeYAdd = 0.0;
                    }
                    break;
                }
                default:
                    ;
                }
            }
            break;
        }
        case PGE_Phys_Object::LVLPlayer:
        {
            LVL_Player *plr = dynamic_cast<LVL_Player *>(cEL);
            SDL_assert(plr);
            l_pushPlr(plr);
            break;
        }
        case PGE_Phys_Object::LVLNPC:
        {
            LVL_Npc *npc = dynamic_cast<LVL_Npc *>(cEL);
            SDL_assert(npc);
            if(!npc ||
               npc->killed ||
               npc->data.friendly ||
               npc->m_isGenerator ||
               !npc->isActivated)
                break;
            l_pushNpc(npc);
            break;
        }
        }
    }

    if(doKill)
        kill(killReason);
    else if(doHurt)
        harm(hurtDamage, killReason);
}

void LVL_Npc::preCollision()
{
    environments_map.clear();
    contacted_bgos.clear();
    contacted_npc.clear();
    contacted_blocks.clear();
    contacted_players.clear();
}

void LVL_Npc::postCollision()
{
    if(m_crushedHard)
        kill(DAMAGE_BY_KICK);
}

void LVL_Npc::collisionHitBlockTop(std::vector<PGE_Phys_Object *> &blocksHit)
{
    (void)(blocksHit);
}

bool LVL_Npc::preCollisionCheck(PGE_Phys_Object *body)
{
    SDL_assert(body);
    switch(body->type)
    {
    case LVLBlock:
    {
        if(this->m_disableBlockCollision)
            return true;
        LVL_Block *blk = static_cast<LVL_Block *>(body);
        if(blk->m_destroyed)
            return true;
    }
    break;
    case LVLNPC:
    {
        if(this->m_disableBlockCollision)
            return true;
    }
    default:
        ;
    }
    return false;
}


bool LVL_Npc::onCliff()
{
    return m_cliff;
}

bool LVL_Npc::onGround()
{
    return m_stand;
}
