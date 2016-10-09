/**********************************************************************
 *
 * Filename: main.cpp
 *
 * Description: Plugin contains common animation nodes and commands.
 *
 * Copyright (C) 2015 Richard Layman, rlayman2000@yahoo.com 
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
 *
 ***********************************************************************/

#include <feather/deps.hpp>
#include <feather/pluginmanager.hpp>
#include <feather/field.hpp>
#include <feather/node.hpp>
#include <feather/parameter.hpp>
#include <feather/command.hpp>
//#include <feather/scenegraph.hpp>
#include <feather/draw.hpp>
#include <feather/plugin.hpp>
#include <feather/tools.hpp>
#include <QColor>

#ifdef __cplusplus
extern "C" {
#endif
    C_PLUGIN_WRAPPER()
#ifdef __cplusplus
}
#endif

using namespace feather;

#define ANIMATION_KEY 420
#define ANIMATION_KEYTRACK 425
#define ANIMATION_MORPH 426


PLUGIN_INIT("Animation","Animation nodes and commands","Richard Layman",ANIMATION_KEY,ANIMATION_MORPH)


/*
 ***************************************
 *            ANIMATION KEY            *
 ***************************************
*/

// time 
ADD_FIELD_TO_NODE(ANIMATION_KEY,FReal,field::Real,field::connection::In,0.0,1)
// int value
ADD_FIELD_TO_NODE(ANIMATION_KEY,FInt,field::Int,field::connection::In,0,2)
// real value
ADD_FIELD_TO_NODE(ANIMATION_KEY,FReal,field::Real,field::connection::In,0.0,3)
// value type 
ADD_FIELD_TO_NODE(ANIMATION_KEY,FInt,field::Int,field::connection::In,field::Real,4)
// in curve type 
ADD_FIELD_TO_NODE(ANIMATION_KEY,FInt,field::Int,field::connection::In,0,5)
// out curve type 
ADD_FIELD_TO_NODE(ANIMATION_KEY,FInt,field::Int,field::connection::In,0,6)
// add curve vectors

namespace feather
{

    DO_IT(ANIMATION_KEY)
    { 
        return status();
    };

    /*
    DRAW_IT(ANIMATION_KEY)
    {
        std::cout << "ANIMATION_KEY DRAW IT\n";
        ADD_SHADED_MESH(3)
        return status();    
    };
    */

} // namespace feather

NODE_INIT(ANIMATION_KEY,node::Animation,"key.svg")


/*
 ***************************************
 *            ANIMATION TRACK          *
 ***************************************
*/

// time 
ADD_FIELD_TO_NODE(ANIMATION_KEYTRACK,FReal,field::Real,field::connection::In,0.0,1)
// keys 
ADD_FIELD_TO_NODE(ANIMATION_KEYTRACK,FNodeArray,field::NodeArray,field::connection::In,FNodeArray(),2)
// value 
ADD_FIELD_TO_NODE(ANIMATION_KEYTRACK,FReal,field::Real,field::connection::Out,0.0,3)

namespace feather
{

    DO_IT(ANIMATION_KEYTRACK)
    { 
        typedef field::Field<FInt>*  IntField;
        typedef field::Field<FReal>*  RealField;
        typedef field::Field<FNodeArray>*  NodeArrayField;

        RealField time;
        NodeArrayField keys;
        RealField value;

        for(auto f : fields){
            if(f->id == 1)
                time = static_cast<RealField>(f);
            if(f->id == 2)
                keys = static_cast<NodeArrayField>(f);
            if(f->id == 3)
                value = static_cast<RealField>(f);
        }

        if(!keys->connections.size()){
            std::cout << "no keys connected\n";
            return status();
        }

        std::cout << keys->connections.size() << " keys connected\n";

        // TODO - I need to find a way to automate this so the plugin dev doesn't miss this step, maybe put this in the update process before the doit()
        if(time->connections.size())
            time->value = static_cast<RealField>(plugin::get_node_field_base(time->connections.at(0).puid,time->connections.at(0).pfid))->value;
        // do the same for the value
        if(value->connections.size())
            value->value = static_cast<RealField>(plugin::get_node_field_base(value->connections.at(0).puid,value->connections.at(0).pfid))->value;
 

        // these are the key values from before and after the input time
        // TODO - currently we're only doing linear calculations
        //FReal minVal=0;
        //FReal minTime=time->value;
        //FReal maxVal=0;
        //FReal maxTime=time->value;

        field::Connection* minConn = nullptr;
        field::Connection* maxConn = nullptr;
        FReal minDeltaTime;
        FReal maxDeltaTime; 
        // get the min and max key connections
        int i = 0;
        for(auto conn : keys->connections){
            IntField keytype = static_cast<IntField>(plugin::get_node_field_base(conn.puid,4));
            RealField keytime = static_cast<RealField>(plugin::get_node_field_base(conn.puid,1));
            std::cout << "key for uid:" << conn.puid << ", time=" << keytime->value << std::endl;
            if(time->value == keytime->value){
                if(keytype->value == field::Int){
                    IntField ival = static_cast<IntField>(plugin::get_node_field_base(conn.puid,2));
                    value->value = static_cast<FReal>(ival->value);
                    std::cout << "ival = " << ival->value;
                    std::cout << "1 KEY TRACK OUT VALUE = " << value->value << std::endl;
                 }
                if(keytype->value == field::Real){
                    RealField rval = static_cast<RealField>(plugin::get_node_field_base(conn.puid,3));
                    value->value = rval->value;
                    std::cout << "rval = " << rval->value;
                    std::cout << "2 KEY TRACK OUT VALUE = " << value->value << std::endl;
                 }
                // we have the value, no need to go any further
                return status();
            }

            else if(keytime->value < time->value) {
                if(!minConn){
                    minDeltaTime = time->value - keytime->value;
                    std::cout << "setting minConn 1: time:" << keytime->value << std::endl;
                    minConn = &keys->connections.at(i);
                } else {
                    if(keytime->value > (time->value - minDeltaTime)){
                        minConn = &keys->connections.at(i);
                        std::cout << "setting minConn 2: time:" << keytime->value << std::endl;
                        minDeltaTime = time->value - keytime->value;
                    }
                }
            }

            else if(keytime->value > time->value) {
                if(!maxConn){
                    maxDeltaTime = keytime->value - time->value;
                    std::cout << "setting maxConn 1: time:" << keytime->value << std::endl;
                    maxConn = &keys->connections.at(i);
                } else {
                    if(keytime->value < (time->value + maxDeltaTime)){
                        maxConn = &keys->connections.at(i);
                        std::cout << "setting maxConn 2: time:" << keytime->value << std::endl;
                        maxDeltaTime = keytime->value - time->value;
                    }
                }
            }
            i++;

            //std::cout << "key for uid:" << conn.puid << ", ival=" << ival->value << ", rval=" << rval->value << ", time=" << time->value << std::endl;
        }

        // If we've made it this far, the value has to be calculated.
        // If there are no keys before the time, we'll use the first key's value
        // If there are no keys after the time, we'll use the last key's value
        // calculate the output value
        if(!minConn){
            IntField keytype = static_cast<IntField>(plugin::get_node_field_base(maxConn->puid,4));
            RealField keytime = static_cast<RealField>(plugin::get_node_field_base(maxConn->puid,1));
            if(time->value == keytime->value){
                if(keytype->value == field::Int){
                    IntField ival = static_cast<IntField>(plugin::get_node_field_base(maxConn->puid,2));
                    value->value = static_cast<FReal>(ival->value);
                    std::cout << "3 KEY TRACK OUT VALUE = " << value->value << std::endl;
                 }
                if(keytype->value == field::Real){
                    RealField rval = static_cast<RealField>(plugin::get_node_field_base(maxConn->puid,3));
                    value->value = rval->value;
                    std::cout << "4 KEY TRACK OUT VALUE = " << value->value << std::endl;
                 }
                // we have the value, no need to go any further
                return status();
            }
        }
        else if(!maxConn){
            IntField keytype = static_cast<IntField>(plugin::get_node_field_base(minConn->puid,4));
            RealField keytime = static_cast<RealField>(plugin::get_node_field_base(minConn->puid,1));
            if(time->value == keytime->value){
                if(keytype->value == field::Int){
                    IntField ival = static_cast<IntField>(plugin::get_node_field_base(minConn->puid,2));
                    value->value = static_cast<FReal>(ival->value);
                    std::cout << "5 KEY TRACK OUT VALUE = " << value->value << std::endl;
                 }
                if(keytype->value == field::Real){
                    RealField rval = static_cast<RealField>(plugin::get_node_field_base(minConn->puid,3));
                    value->value = rval->value;
                    std::cout << "6 KEY TRACK OUT VALUE = " << value->value << std::endl;
                 }
                // we have the value, no need to go any further
                return status();
            }
        }
        else {
            IntField maxkeytype = static_cast<IntField>(plugin::get_node_field_base(minConn->puid,4));
            IntField minkeytype = static_cast<IntField>(plugin::get_node_field_base(maxConn->puid,4));
            RealField minkeytime = static_cast<RealField>(plugin::get_node_field_base(minConn->puid,1));
            RealField maxkeytime = static_cast<RealField>(plugin::get_node_field_base(maxConn->puid,1));
            FReal minvalue;
            FReal maxvalue;

            // min value
            if(minkeytype->value == field::Int){
                IntField ival = static_cast<IntField>(plugin::get_node_field_base(minConn->puid,2));
                minvalue = static_cast<FReal>(ival->value);
            }
            if(minkeytype->value == field::Real){
                RealField rval = static_cast<RealField>(plugin::get_node_field_base(minConn->puid,3));
                minvalue = rval->value;
            }

            // max value
            if(maxkeytype->value == field::Int){
                IntField ival = static_cast<IntField>(plugin::get_node_field_base(maxConn->puid,2));
                maxvalue = static_cast<FReal>(ival->value);
            }
            if(maxkeytype->value == field::Real){
                RealField rval = static_cast<RealField>(plugin::get_node_field_base(maxConn->puid,3));
                maxvalue = rval->value;
            }
        
            // do a single rise run calculation to set the out value
            FReal rise = maxvalue - minvalue;
            FReal run = maxkeytime->value - minkeytime->value;
            FReal slope = rise/run;
            value->value = minvalue + ((time->value - minkeytime->value) * slope);            
            std::cout << "On step 7, "
                << " minVal:" << minvalue
                << " minTime:" << minkeytime->value
                << " maxVal:" << maxvalue
                << " maxTime:" << maxkeytime->value
                << " rise:" << rise
                << " run:" << run
                << " slope:" << slope
                << std::endl
                ;
            std::cout << "7 KEY TRACK OUT VALUE = " << value->value << std::endl;
            value->update = true;
        }

        return status();
    };

} // namespace feather

NODE_INIT(ANIMATION_KEYTRACK,node::Animation,"track.svg")



/*
 ***************************************
 *           ANIMATION MORPH           *
 ***************************************
*/

// FIELDS

// IN

// base 
ADD_FIELD_TO_NODE(ANIMATION_MORPH,FMesh,field::MeshArray,field::connection::In,FMesh(),1)
// targets 
ADD_FIELD_TO_NODE(ANIMATION_MORPH,FMeshArray,field::MeshArray,field::connection::In,FMeshArray(),2)
// weights
ADD_FIELD_TO_NODE(ANIMATION_MORPH,FRealArray,field::Real,field::connection::In,std::vector<FReal>(),3)

// OUT

// mesh 
ADD_FIELD_TO_NODE(ANIMATION_MORPH,FMesh,field::Mesh,field::connection::Out,FMesh(),4)

namespace feather
{

    DO_IT(ANIMATION_MORPH)
    { 
        // get fields
        GET_FIELD_DATA(1,FMesh,baseIn,field::connection::In)
        GET_FIELD_ARRAY_DATA(2,FMeshArray,targetsIn,field::connection::In)
        GET_FIELD_ARRAY_DATA(3,FRealArray,weightsIn,field::connection::In)
        GET_FIELD_DATA(4,FMesh,meshOut,field::connection::Out)

        if(weightsIn->update)
        {
            // clear the meshOut
            meshOut->value.v.clear();
            meshOut->value.st.clear();
            meshOut->value.vn.clear();
            meshOut->value.f.clear();
            meshOut->update = true;
           
            if(!baseIn->connections.size())
                return status(FAILED,"no target meshes in morph node.");

            // if we've made it this far, we will blend the mesh to the final mesh
            meshOut->value = baseIn->value;
 
            // if there is only the base mesh or there are no weights, just output that
            if(!targetsIn->connections.size())
            {
                return status();
            }
   
            uint id=0;

            for(auto conn : targetsIn->connections)
            {
                FMesh* pMesh = &static_cast<field::Field<FMesh>*>(plugin::get_node_field_base(conn.puid,conn.pfid))->value;
                FReal weight=0.0;
                if(!weightsIn->connections.size() || weightsIn->connections.size() < id){
                    if(weightsIn->value.size() <= id) {
                        weight = 0.0;
                        weightsIn->value.push_back(weight);
                    } else {
                        weight = weightsIn->value[id];
                    }
                } else {
                    // use the connection weight value
                    weight = static_cast<field::Field<FReal>*>(plugin::get_node_field_base(weightsIn->connections.at(id).puid,weightsIn->connections.at(id).pfid))->value;
                }
                for(uint i=0; i < meshOut->value.v.size(); i++){
                    meshOut->value.v.at(i).x = meshOut->value.v.at(i).x + ( ( pMesh->v.at(i).x - baseIn->value.v.at(i).x ) * weight );
                    meshOut->value.v.at(i).y = meshOut->value.v.at(i).y + ( ( pMesh->v.at(i).y - baseIn->value.v.at(i).y ) * weight );
                    meshOut->value.v.at(i).z = meshOut->value.v.at(i).z + ( ( pMesh->v.at(i).z - baseIn->value.v.at(i).z ) * weight );
                }
                weight=0.0;
                id++; 
            } 
        }

        return status();
    };

    /*
    DRAW_IT(ANIMATION_MORPH)
    {
        std::cout << "ANIMATION_MORPH DRAW IT\n";
        ADD_SHADED_MESH(3)
        return status();    
    };
    */

} // namespace feather

NODE_INIT(ANIMATION_MORPH,node::Animation,"morph.svg")



/*
 ***************************************
 *              COMMANDS               *
 ***************************************
*/

namespace feather
{
    namespace command
    {
        enum Command { N=0, ADD_KEY };

        // add key
        status add_key(parameter::ParameterList params) {
            double time; 
            bool pass = params.getParameterValue<double>("time",time);
            if(!pass)
                return status(FAILED,"time parameter failed");

            // add key to scenegraph
            status p;
            unsigned int uid = plugin::add_node(ANIMATION_KEY,"key",p);

            // set the time of the key
            typedef field::Field<feather::FReal>* sourcefield;
            sourcefield sf=NULL;
            sf = static_cast<sourcefield>(feather::plugin::get_node_field_base(uid,1));
            sf->value = time;

            // connect to the root for now
            plugin::connect(0,202,uid,201);
        
            // update scenegraph            
            plugin::update();
            
            std::cout << "add_key command found\n";
 
            return status();
        };

    } // namespace command

} // namespace feather

ADD_COMMAND("add_key",ADD_KEY,add_key)
ADD_PARAMETER(command::ADD_KEY,1,parameter::Real,"time")

INIT_COMMAND_CALLS(ADD_KEY)
