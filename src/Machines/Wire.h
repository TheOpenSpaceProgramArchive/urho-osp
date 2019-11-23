#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>

#include "Machine.h"

namespace osp {

// Add, Remove, and Rename a bunch of these
enum WireType
{
    WIRE_NONE,       //
    WIRE_INT,        //
    WIRE_FLOAT,      //
    WIRE_POWER,      // Combined On, Off, or Toggle. ie. Ignite, Shutdown
    WIRE_ROT,        // A Quaternion in global space
    WIRE_ROT_ATT,    // A Quaternion in planet space (attitude)
    WIRE_ROT_ACTION, // Change in rotation in Eular ZXY (Roll, Pitch, Yaw)
    WIRE_PERCENT,    // ie. Throttle control
    //WIRE_SATELLITE,  // ie. Target a satellite?
    WIRE_SERIAL,     // not sure what to use this for
    WIRE_STRING,     // not sure what to use this for either
    WIRE_VARIANT     // custom, get type by m_dataOut.GetType()
};


class Machine;
class WireInput;
class WireOutput;

/**
 * Used as member variables of a machine to get controls and information from
 * a WireOutput
 */
class WireInput : public Urho3D::RefCounted
{

    friend WireOutput;

public:

    WireInput(Machine* mach, Urho3D::String const& name);
    ~WireInput() = default;

    // TODO: add some convinient functions;
    float recieve_percentf();

    void Connect(WireOutput* data);

    inline Urho3D::String const get_name() const { return m_name; }
    inline Urho3D::StringHash get_nameHash() const { return m_nameHash; }

protected:

    // this is a suggestion, this input can be connected to any type of output
    WireType m_type;

    Urho3D::String m_name;
    Urho3D::StringHash m_nameHash;

    Urho3D::WeakPtr<Machine> m_machine; // Associated machine
    Urho3D::WeakPtr<WireOutput> m_dataIn;


    
    // put stuff here
    
};

inline WireInput::WireInput(Machine* mach,
                            Urho3D::String const& name):
        Urho3D::RefCounted()
{
    m_machine = mach;
    m_name = name;
    m_nameHash = name;
}

/**
 * Used to output information from a Machine, to feed into another Machine's
 * WireInputs.
 */
class WireOutput : public Urho3D::RefCounted
{

    friend WireInput;

public:

   WireOutput(Machine* mach, Urho3D::String const& name);
   ~WireOutput()
   {
       // call destructor of whatever is in the union
   };

   void send_percentf(float precent);

   inline WireType get_wire_type()
   {
       return m_type;
   }

   inline Urho3D::String const get_name() const { return m_name; }
   inline Urho3D::StringHash get_nameHash() const { return m_nameHash; }

protected:

   Urho3D::Vector< WeakPtr<WireInput> > m_connectedInputs;
   Urho3D::WeakPtr<Machine> m_machine;
   WireType m_type;

   Urho3D::String m_name;
   Urho3D::StringHash m_nameHash;

   union
   {
       //PODVector<unsigned char> m_serialOut;
       Urho3D::Variant m_data;

       // OSP-specific outputs
   };


   // put stuff here
};


inline WireOutput::WireOutput(Machine* mach,
                            Urho3D::String const& name) :
        Urho3D::RefCounted()
{
    m_machine = mach;
    m_name = name;
    m_nameHash = name;
}

}
