// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file    protocols/normalmode/NormalModeRelaxMover.hh
/// @brief   initialization for NormalMode
/// @details
/// @author  Hahnbeom Park

#ifndef INCLUDED_protocols_normalmode_NormalModeRelaxMover_hh
#define INCLUDED_protocols_normalmode_NormalModeRelaxMover_hh

// Unit headers
#include <protocols/normalmode/NormalMode.hh>
#include <protocols/normalmode/NormalModeRelaxMover.fwd.hh>

// Package headers
#include <protocols/moves/Mover.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/optimization/MinimizerOptions.hh>

// Project headers
#include <core/pose/Pose.fwd.hh>

//// C++ headers
#include <string>

#include <core/kinematics/MoveMap.fwd.hh>
#include <core/select/movemap/MoveMapFactory.fwd.hh>
#include <utility/vector1.hh>
#include <utility>

namespace protocols {
namespace normalmode {

class NormalModeRelaxMover : public protocols::moves::Mover
{
public:
	NormalModeRelaxMover();

	NormalModeRelaxMover( core::scoring::ScoreFunctionCOP sfxn,
		bool const cartesian );

	NormalModeRelaxMover( core::scoring::ScoreFunctionCOP sfxn,
		bool cartesian,
		core::kinematics::MoveMapCOP mm,
		std::string const relaxmode,
		core::Real const distcut
	);

	~NormalModeRelaxMover() override;


	protocols::moves::MoverOP
	clone() const override { return utility::pointer::make_shared< NormalModeRelaxMover >(*this); }


	void apply( core::pose::Pose & pose ) override;

	void apply_on_pose( core::pose::Pose & pose, core::kinematics::MoveMapOP movemap );

	/// Virtual functions from Mover
	protocols::moves::MoverOP fresh_instance() const override { return clone(); };


	void parse_my_tag( TagCOP tag,
		basic::datacache::DataMap & data
	) override;

	void set_default();

	/// Setters
	void set_harmonic_constants( core::Real const k_uniform );

	void set_harmonic_constants( core::Real const k_connected,
		core::Real const k_segment,
		core::Real const k_long );

	void invert_direction(){ direction_ *= -1; }

	void set_extrapolate_scale( core::Real const scale ){ moving_distance_ = scale; }

	void set_cst_sdev( core::Real const cst_sdev ){ cst_sdev_ = cst_sdev; }

	// Use this to modify minimizer; for example to run AtomTreeMin in CartRelaxMover, set as false
	void set_cartesian_minimize( bool const value ){ cartesian_minimize_ = value; }

	void set_minoption( core::optimization::MinimizerOptionsCOP minoption ){ minoption_ = minoption->clone(); }

	void set_relaxmode( std::string const mode ){ relaxmode_=mode; }

	// Mode setup;
	// user-defined mode setup
	void set_mode( utility::vector1< core::Size > const mode_using,
		utility::vector1< core::Real > const mode_scales );

	void set_mode( core::Size const i_mode );

	void set_random_mode( std::string const select_option = "probabilistic",
		core::Real const importance_portion = 1.0 );

	void refresh_normalmode() { refresh_normalmode_ = true; }

	// Accessors
	protocols::normalmode::NormalMode const NM() const { return NM_; }
	core::Real cst_sdev() const { return cst_sdev_; }
	core::Real get_dynamic_scale() const { return scale_dynamic_; }
	std::string relaxmode() const { return relaxmode_; }

	std::string
	get_name() const override;

	///@brief used by the job distributor and MultiplePoseMover (MPM) to return as many of the potential poses as possible.
	core::pose::PoseOP
	get_additional_output() override;

	static
	std::string
	mover_name();

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );


private:
	virtual
	void initialize_NM_for_movemap(
		core::pose::Pose const & pose,
		core::kinematics::MoveMap const & movemap );

	void
	gen_coord_constraint( core::pose::Pose &pose,
		utility::vector1< core::Vector > const &excrd ) const;

	void set_default_minoption();

	utility::vector1< core::Vector >
	extrapolate_mode_on_crd( core::pose::Pose const &pose ) const;

	core::pose::Pose
	extrapolate_mode_on_pose( core::pose::Pose const &pose ) const;

	core::Real get_RMSD( utility::vector1< core::Vector > const excrd,
		core::pose::Pose const &pose ) const;


private:
	core::Size nmodes_ = 5;
	bool mix_modes_ = false;
	bool cartesian_ = true;
	core::Real pertscale_ = 1.0;
	bool randomselect_ = false;
	core::Real selection_kT_ = 1e6; // currently just placeholder
	bool centroid_ = false;
	core::Size nsample_ = 5;
	core::Real moving_distance_ = 1.0;
	bool refresh_normalmode_ = true;
	core::Real direction_ = 1.0;
	core::Real cst_sdev_ = 1.0;
	bool cartesian_minimize_ = false;
	bool dump_silent_ = false;
	std::string outsilent_;

	utility::vector1< core::pose::PoseOP > output_pose_OPs_;

	std::string relaxmode_;
	core::optimization::MinimizerOptionsCOP minoption_;

	utility::vector1< core::Size > mode_using_;
	utility::vector1< core::Real > mode_scale_;

	core::select::movemap::MoveMapFactoryCOP mmf_;
	core::kinematics::MoveMapOP mm_;
	core::scoring::ScoreFunctionOP sfxn_;
	core::scoring::ScoreFunctionOP sfxn_cen_;
	protocols::normalmode::NormalMode NM_;

	// dynamic ones
	mutable core::Real scale_dynamic_;

	// torsion stuffs
	mutable utility::vector1< core::Real > dtor_;

}; // end NormalModeRelaxMover

} // normalmode
} // protocols

#endif
