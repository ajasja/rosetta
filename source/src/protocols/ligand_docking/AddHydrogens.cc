// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   protocols/ligand_docking/AddHydrogens.cc
///
/// @brief
/// @Gordon Lemmon

#include <protocols/ligand_docking/AddHydrogens.hh>
#include <protocols/ligand_docking/AddHydrogensCreator.hh>
#include <protocols/ligand_docking/AddHydrogen.hh>
#include <protocols/ligand_docking/LigandDesign.hh>

#include <core/pose/chains_util.hh>

#include <core/types.hh>

#include <basic/Tracer.hh>

// option key includes
#include <utility/tag/Tag.hh>

#include <utility/vector1.hh>

//Auto Headers
#include <core/conformation/Conformation.hh>
#include <core/pose/Pose.hh>
// XSD XRW Includes
#include <utility/tag/XMLSchemaGeneration.hh>
#include <protocols/moves/mover_schemas.hh>


namespace protocols {
namespace ligand_docking {

static basic::Tracer add_hydrogens_tracer( "protocols.ligand_docking.LigandDesign", basic::t_debug );




AddHydrogens::AddHydrogens():
	//utility::VirtualBase(),
	Mover("AddHydrogens"),
	chain_("X")
{
	Mover::type( "AddHydrogens" );
}

AddHydrogens::AddHydrogens(AddHydrogens const & /*that*/) = default;

AddHydrogens::~AddHydrogens() = default;

protocols::moves::MoverOP AddHydrogens::clone() const {
	return utility::pointer::make_shared< AddHydrogens >( *this );
}

protocols::moves::MoverOP AddHydrogens::fresh_instance() const {
	return utility::pointer::make_shared< AddHydrogens >();
}


/// @brief parse XML (specifically in the context of the parser/scripting scheme)
void
AddHydrogens::parse_my_tag(
	utility::tag::TagCOP tag,
	basic::datacache::DataMap & /*datamap*/
)
{
	debug_assert( tag->getName() != "AddHydrogens");
	if ( ! tag->hasOption("chain") ) utility_exit_with_message("'AddHydrogens' requires 'chain' tag");
	chain_ = tag->getOption<std::string>("chain");
}

void
AddHydrogens::apply( core::pose::Pose & pose )
{
	core::Size const chain_id= core::pose::get_chain_id_from_chain(chain_, pose);
	core::Size const chain_begin= pose.conformation().chain_begin(chain_id);
	core::Size const chain_end= pose.conformation().chain_end(chain_id);
	utility::vector1<core::Size> unconnected_ids= find_unconnected_residues(pose, chain_begin, chain_end);

	for ( core::Size const unconnected_id : unconnected_ids ) {
		core::conformation::Residue const & res_to_fix= pose.residue(unconnected_id);
		utility::vector1<core::Size> connect_ids= get_incomplete_connections(res_to_fix.get_self_ptr());

		for ( core::Size const connect_id : connect_ids ) {
			AddHydrogen AH(unconnected_id, connect_id);
			AH.apply(pose);
		}
	}
}

std::string AddHydrogens::get_name() const {
	return mover_name();
}

std::string AddHydrogens::mover_name() {
	return "AddHydrogens";
}

void AddHydrogens::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd )
{
	using namespace utility::tag;
	AttributeList attlist;
	attlist + XMLSchemaAttribute::required_attribute("chain", xs_string, "PDB-file chain ID");
	protocols::moves::xsd_type_definition_w_attributes( xsd, mover_name(), "Saturates the incomplete connections with H", attlist );
}

std::string AddHydrogensCreator::keyname() const {
	return AddHydrogens::mover_name();
}

protocols::moves::MoverOP
AddHydrogensCreator::create_mover() const {
	return utility::pointer::make_shared< AddHydrogens >();
}

void AddHydrogensCreator::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd ) const
{
	AddHydrogens::provide_xml_schema( xsd );
}


} // namespace ligand_docking
} // namespace protocols
