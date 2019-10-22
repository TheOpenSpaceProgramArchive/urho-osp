#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>

#include "Machine.h"

namespace osp {

enum WireType
{
    WIRE_INT,       //
    WIRE_FLOAT,     //
    WIRE_CLOCK,     //
    WIRE_PERCENT,   // ie. Throttle control
    WIRE_SATELLITE, // ie. Target a satellite?
    WIRE_SERIAL,    // not sure what to use this for
    WIRE_STRING,    // not sure what to use this for either
    WIRE_VARIANT    // custom, get type by m_dataOut.GetType()
};


class Machine;
class WireInput;
class WireOutput;

/**
 * Used as member variables of a machine to get controls and information from
 * a WireOutput
 */
class WireInput : public Urho3D::Object
{
    URHO3D_OBJECT(WireInput, Urho3D::Object)

public:

    using Urho3D::Object::Object;
    ~WireInput() = default;

protected:

    Urho3D::WeakPtr<Machine> m_machine; // Associated machine
    Urho3D::WeakPtr<WireOutput> m_dataIn;
    
    // put stuff here
    
};

/**
 * @brief The WireInput class
 */
class WireOutput : public Urho3D::Object
{
   URHO3D_OBJECT(WireOutput, Urho3D::Object)

public:

   using Urho3D::Object::Object;
   ~WireOutput()
   {
       // call destructor of whatever is in the union
   };

protected:

   Urho3D::Vector< WeakPtr<WireInput> > m_connectedInputs;
   Urho3D::WeakPtr<Machine> m_machine;
   WireType m_type;

   union
   {
       //PODVector<unsigned char> m_serialOut;
       Urho3D::Variant m_dataOut;

       // OSP-specific outputs
   };


   // put stuff here
};


}
