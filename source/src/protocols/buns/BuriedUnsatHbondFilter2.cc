// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/buns/BuriedUnsatHbondFilter2.cc
/// @brief
/// @author Kevin Houlihan (khouli@unc.edu)

#include <protocols/buns/BuriedUnsatHbondFilter2.hh>
#include <protocols/buns/BuriedUnsatHbondFilter2Creator.hh>

#include <core/pose/Pose.hh>
// PDBInfo for debug
#include <core/pose/PDBInfo.hh>
#include <basic/Tracer.hh>
#include <utility/tag/Tag.hh>
#include <basic/datacache/DataMap.hh>
#include <protocols/rosetta_scripts/util.hh>
#include <protocols/rigid/RigidBodyMover.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <basic/MetricValue.hh>
#include <protocols/simple_pose_metric_calculators/BuriedUnsatisfiedPolarsCalculator.hh>
#include <protocols/buns/BuriedUnsatisfiedPolarsCalculator2.hh>
#include <core/pack/task/TaskFactory.fwd.hh>

// XSD XRW Includes
#include <utility/tag/XMLSchemaGeneration.hh>
#include <protocols/filters/filter_schemas.hh>

namespace protocols {
namespace buns {

static basic::Tracer TR( "protocols.buns.BuriedUnsatHbondFilter2" );



BuriedUnsatHbondFilter2::BuriedUnsatHbondFilter2( core::Size const upper_threshold, core::Size const jump_num ) :
	protocols::filters::Filter( "BuriedUnsatHbonds2" ),
	upper_threshold_( upper_threshold ),
	jump_num_( jump_num ),
	task_factory_( /* NULL */ ),
	calc_( /* NULL */)
{ }

// @brief copy constructor
BuriedUnsatHbondFilter2::BuriedUnsatHbondFilter2( BuriedUnsatHbondFilter2 const & rval ):
	Filter( rval ),
	sfxn_( rval.sfxn_ ),
	upper_threshold_( rval.upper_threshold_ ),
	jump_num_( rval.jump_num_ ),
	task_factory_( rval.task_factory_ ),
	calc_( rval.calc_ )
{}


BuriedUnsatHbondFilter2::BuriedUnsatHbondFilter2() : protocols::filters::Filter( "BuriedUnsatHbonds2" ) {}

BuriedUnsatHbondFilter2::~BuriedUnsatHbondFilter2() = default;

void
BuriedUnsatHbondFilter2::parse_my_tag( utility::tag::TagCOP tag, basic::datacache::DataMap & datamap )
{
	jump_num_ = tag->getOption<core::Size>( "jump_number", 1 );
	upper_threshold_ = tag->getOption<core::Size>( "cutoff", 20 );

	// calc_ = utility::pointer::make_shared< protocols::buns::BuriedUnsatisfiedPolarsCalculator2 >("default");
	//  sboyken NEED DIFFERENT NAME THAN "default" OTHERWISE DOES NOT PLAY NICE WHEN CALL MULTIPLE UNSAT FILTERS WITHIN SAME XML!!!
	calc_ = utility::pointer::make_shared< protocols::buns::BuriedUnsatisfiedPolarsCalculator2 >("bunsat_calc2_default_weak_bunsat_calc");

	if ( tag->hasOption("generous_hbonds") ) {
		calc_->set_generous_hbonds(tag->getOption<bool>( "generous_hbonds", true  ));
	}
	if ( tag->hasOption("sasa_burial_cutoff") ) {
		calc_->set_sasa_burial_cutoff( tag->getOption<core::Real>( "sasa_burial_cutoff" ) );
	}
	if ( tag->hasOption("AHD_cutoff") ) {
		calc_->set_AHD_cutoff(tag->getOption<core::Real>( "AHD_cutoff", 120.0 ));
	}
	if ( tag->hasOption("dist_cutoff") ) {
		calc_->set_dist_cutoff(tag->getOption<core::Real>( "dist_cutoff", 3.0 ));
	}
	if ( tag->hasOption("hxl_dist_cutoff") ) {
		calc_->set_hxl_dist_cutoff(tag->getOption<core::Real>( "hxl_dist_cutoff", 3.5 ));
	}
	if ( tag->hasOption("sulph_dist_cutoff") ) {
		calc_->set_sulph_dist_cutoff(tag->getOption<core::Real>( "sulph_dist_cutoff", 3.3 ));
	}
	if ( tag->hasOption("metal_dist_cutoff") ) {
		calc_->set_metal_dist_cutoff(tag->getOption<core::Real>( "metal_dist_cutoff", 2.7 ));
	}

	std::string const scorefxn_key( protocols::rosetta_scripts::get_score_function_name(tag) );
	if ( datamap.has( "scorefxns", scorefxn_key ) ) {
		sfxn_ = datamap.get_ptr<core::scoring::ScoreFunction>( "scorefxns", scorefxn_key );
	} else {
		sfxn_ = core::scoring::get_score_function();
	}
	task_factory( protocols::rosetta_scripts::parse_task_operations( tag, datamap ) );

	TR<<"Buried Unsatisfied Hbond filter v2 over jump number " << jump_num_ << " with cutoff " << upper_threshold_ << std::endl;
}

bool
BuriedUnsatHbondFilter2::apply( core::pose::Pose const & pose ) const {
	core::Size const unsat_hbonds( compute( pose ) );

	TR<<"# unsatisfied hbonds: "<<unsat_hbonds<<". ";
	if ( unsat_hbonds <= upper_threshold_ ) {
		TR<<"passing." <<std::endl;
		return true;
	} else {
		TR<<"failing."<<std::endl;
		return false;
	}
}

void
BuriedUnsatHbondFilter2::report( std::ostream & out, core::pose::Pose const & pose ) const {
	core::Size const unsat_hbonds( compute( pose ));
	out<<"# unsatisfied hbonds: "<< unsat_hbonds<<'\n';
}

core::Real
BuriedUnsatHbondFilter2::report_sm( core::pose::Pose const & pose ) const {
	core::Real const unsat_hbonds( compute( pose ));
	return( static_cast< core::Real >(unsat_hbonds) );
}

core::Size
BuriedUnsatHbondFilter2::compute( core::pose::Pose const & pose ) const {

	if ( pose.pdb_info() != nullptr ) {
		TR << "PDB: " << pose.pdb_info()->name() << std::endl;
	}

	runtime_assert( jump_num_ <= pose.num_jump() );

	// score the pose to populate the 10A neighborgraph
	core::pose::Pose bound( pose );

	core::scoring::ScoreFunctionOP scorefxn = core::scoring::get_score_function();
	( *scorefxn )( bound );
	bound.update_residue_neighbors();

	using namespace protocols::simple_pose_metric_calculators;

	basic::MetricValue< core::Size > total_buns_bound;
	basic::MetricValue< utility::vector1< core::Size > > res_buns_bound;

	// guarantee that calculator recomputes
	calc_->notify_energy_change();
	calc_->get("all_bur_unsat_polars", total_buns_bound, bound);
	calc_->get("residue_bur_unsat_polars", res_buns_bound, bound);

	// unbound initialized as vector of 0
	basic::MetricValue< core::Size > total_buns_unbound(0);
	basic::MetricValue< utility::vector1< core::Size > >
		res_buns_unbound(utility::vector1< core::Size >(pose.size(), 0));

	// if given a jump_num, generate an "unbound" pose, use it to fill
	// res_buns_unbound vector
	if ( jump_num_ ) {
		core::pose::Pose unbound( bound );
		core::Real const unbound_dist = 1000.0;
		protocols::rigid::RigidBodyTransMover trans_mover( unbound, static_cast<int>(jump_num_) );
		trans_mover.trans_axis( trans_mover.trans_axis() );
		trans_mover.step_size(unbound_dist);
		trans_mover.apply( unbound );
		//unbound.update_residue_neighbors();
		(*sfxn_)(unbound ); // score the new pose, or we get assertion erros.
		calc_->notify_energy_change();
		calc_->get("all_bur_unsat_polars", total_buns_unbound, unbound);
		calc_->get("residue_bur_unsat_polars", res_buns_unbound, unbound);
	}

	core::Size unsat_hbonds( 0 );

	if ( task_factory_ == nullptr ) {
		unsat_hbonds = total_buns_bound.value() - total_buns_unbound.value();
	} else {
		utility::vector1< core::Size > const selected_residues(
			protocols::rosetta_scripts::residue_packer_states(
			pose, task_factory(), true/*designable*/, true/*packable*/ ) );
		if ( selected_residues.size() == 0 ) {
			unsat_hbonds = 0;
		} else {
			for ( core::Size const sr : selected_residues ) {
				unsat_hbonds += std::max( (res_buns_bound.value()[ sr ]) - (res_buns_unbound.value()[ sr ]), core::Size(0) );
				//TR << "running value for total_in_selected_residues = " << total_in_selected_residues << std::endl;
			}
		}
	}
	return unsat_hbonds;
}

//protocols::filters::FilterOP BuriedUnsatHbondFilter2::clone() const {
// return utility::pointer::make_shared< BuriedUnsatHbondFilter2 >( *this );
//}
//
//protocols::filters::FilterOP BuriedUnsatHbondFilter2::fresh_instance() const{
// return utility::pointer::make_shared< BuriedUnsatHbondFilter2 >();
//}

void
BuriedUnsatHbondFilter2::task_factory( core::pack::task::TaskFactoryOP tf ){
	task_factory_ = tf;
}

core::pack::task::TaskFactoryOP
BuriedUnsatHbondFilter2::task_factory() const {
	return task_factory_;
}

std::string BuriedUnsatHbondFilter2::name() const {
	return class_name();
}

std::string BuriedUnsatHbondFilter2::class_name() {
	return "BuriedUnsatHbonds2";
}

void BuriedUnsatHbondFilter2::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd )
{
	using namespace utility::tag;
	AttributeList attlist;
	attlist + XMLSchemaAttribute::attribute_w_default( "jump_number", xsct_non_negative_integer, "Jump, numbered sequentially from 1, that defines the interface", "1" )
		+ XMLSchemaAttribute::attribute_w_default( "cutoff", xsct_non_negative_integer, "Upper threshold beyond which the filter fails", "20" )
		+ XMLSchemaAttribute::attribute_w_default( "generous_hbonds", xsct_rosetta_bool, "Use a generous definition of H-bonds", "true" )
		+ XMLSchemaAttribute::attribute_w_default( "sasa_burial_cutoff", xsct_real, "SASA threshold to define residue burial", "0.01" )
		+ XMLSchemaAttribute::attribute_w_default( "AHD_cutoff", xsct_real, "Cutoff for AHD angle, in degrees, to define a hydrogen bond", "120" )
		+ XMLSchemaAttribute::attribute_w_default( "dist_cutoff", xsct_real, "Cutoff for distance, in Angstroms, to define a hydrogen bond", "3.0" )
		+ XMLSchemaAttribute::attribute_w_default( "hxl_dist_cutoff", xsct_real, "Cutoff for distance, in Angstroms, to define a hydrogen bond to a hydroxyl", "3.5" )
		+ XMLSchemaAttribute::attribute_w_default( "sulph_dist_cutoff", xsct_real, "Cutoff for distance, in Angstroms, to define a hydrogen bond to a sulfur", "3.3" )
		+ XMLSchemaAttribute::attribute_w_default( "metal_dist_cutoff", xsct_real, "Cutoff for distance, in Angstroms, to define a hydrogen bond to a metal", "2.7" );
	protocols::rosetta_scripts::attributes_for_get_score_function_name( attlist );
	protocols::rosetta_scripts::attributes_for_parse_task_operations( attlist );

	protocols::filters::xsd_type_definition_w_attributes( xsd, class_name(), "A filter for computing the number of buried unsatisfied hydrogen bond donors and/or acceptors, and discarding poses with too many.  This is the same thing that the BuriedUnsatHbonds filter does, but this filter does it a somewhat different way.  What the difference is has been lost to the mists of time.", attlist );
}

std::string BuriedUnsatHbondFilter2Creator::keyname() const {
	return BuriedUnsatHbondFilter2::class_name();
}

protocols::filters::FilterOP
BuriedUnsatHbondFilter2Creator::create_filter() const {
	return utility::pointer::make_shared< BuriedUnsatHbondFilter2 >();
}

void BuriedUnsatHbondFilter2Creator::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd ) const
{
	BuriedUnsatHbondFilter2::provide_xml_schema( xsd );
}


}
}
