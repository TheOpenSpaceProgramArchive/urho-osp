#include <Urho3D/IO/Log.h>

#include "Satellite.h"

using namespace osp;

Satellite::~Satellite()
{
    URHO3D_LOGINFOF("satellite destroyed!");
}

void Satellite::add_child(Satellite *newChild)
{
    if (newChild->m_parent.NotNull())
    {
        // TODO: move old parent's UniquePtr child to this satellite
    }
    newChild->m_index = m_children.Size();
    newChild->m_depth = m_depth + 1;
    newChild->m_parent = this;
    m_children.Push(UniquePtr<Satellite>(newChild));
}


LongVector3 Satellite::calculate_relative_position(const Satellite* from,
                                                const Satellite* to,
                                                unsigned unitsPerMeter)
{
    // Distance to self will never be greater than or less than zero.
    if (from == to)
    {
        //URHO3D_LOGERRORF("Satellite calculating own position");
        return LongVector3::ZERO;
    }
    
    // Special case where their parents are the same
    //if (from->m_parent == to->m_parent)
    //{
    //    
    //}
    
    // Relative position for satellites with different parents
    // Similar to "N-ary Tree Least Common Ancestor"
    // TODO: account for differing m_unitsPerMeter and integer over/underflows
    
    const Satellite* satA = from;
    const Satellite* satB = to;

    LongVector3 posA;
    LongVector3 posB;

    while (satA != satB)
    {
        if (satA->m_depth <= satB->m_depth)
        {
            posB += satB->m_position;
            satB = satB->m_parent;
        }
        else
        {
            posA += satA->m_position;
            satA = satA->m_parent;
        }
        
    }
    
    return posB - posA;
}
