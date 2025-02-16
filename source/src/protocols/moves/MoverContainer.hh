// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file MoverContainer.hh
/// @brief base class for containers of movers such as SequenceMover
/// @author Monica Berrondo


#ifndef INCLUDED_protocols_moves_MoverContainer_hh
#define INCLUDED_protocols_moves_MoverContainer_hh

// Unit headers
#include <protocols/moves/MoverContainer.fwd.hh>
#include <protocols/moves/Mover.hh>

// Package headers
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>

#include <utility/tag/Tag.fwd.hh>
#include <basic/datacache/DataMap.fwd.hh>


// C++ Headers
#include <string>

// Utility Headers
#include <utility/vector0.hh>
#include <utility/vector1.hh>

namespace protocols {
namespace moves {

// A MoverContainer is an array of movers.  Movers placed inside MoverContainers must be cloneable.
// you can either apply them randomly using a RandomOneMover
// or sequentially using a SequenceMover (see .cc for implementation)
// @todo relative weighting of the moves should be handled by this class
class MoverContainer : public Mover {
public:

	// constructor with arguments
	MoverContainer() : Mover() {}

	/// @brief to be called by derived-class clone() methods; copy all data from the source MoverContainer
	MoverContainer( MoverContainer const & source );

	/// @brief Adds a mover to the end of this container
	void add_mover( MoverOP mover_in, core::Real weight_in = 1.0 );

	std::string get_mover( core::Size index) const;

	void clear(){ movers_.clear(); };

	/// @brief Sets the input Pose for both the container
	/// and the contained movers, for rmsd
	void set_input_pose( PoseCOP pose ) override;

	// @brief Sets the native pose for both the container
	// and the contained movers, for rmsd
	void set_native_pose( PoseCOP pose ) override;

	// @brief Sets the current tag for both the container
	// and the contained movers
	void set_current_tag( std::string const & new_tag ) override;

	void apply( core::pose::Pose & pose ) override = 0;
	core::Size nr_moves() { return movers_.size(); };
	core::Size size() const { return movers_.size(); };

	MoverOP front() { return movers_.front(); };

	utility::vector0< core::Real > const & weights() const {
		return weight_;
	}

	utility::vector0< MoverOP > const & movers() const {
		return movers_;
	}

	friend std::ostream &operator<< (std::ostream &os, MoverContainer const &mover);

public: //Functions needed for the citation manager

	/// @brief Provide the citation.
	void provide_citation_info(basic::citation_manager::CitationCollectionList & ) const override;

protected:
	// the weight is only used for RandomMover to pick which one is used, can this be changed?
	utility::vector0< core::Real > weight_;           ///< relative weight when part of MoverContainer.
	utility::vector0< MoverOP > movers_;

}; // MoverContainer class

/// @brief A Mover that iterates through a vector of Movers,
/// applying each one sequentially
///
/// Common Methods:
///     SequenceMover.add_mover
///     SequenceMover.apply
class SequenceMover : public MoverContainer {
public:

	// constructor
	/// @brief Constructs a SequenceMover, optionally setting to use mover status.
	/// seqmover = SequenceMover()

	SequenceMover( bool ms=false );

	/// @brief Convenience constructor: initial sequence of 2 movers
	/// seqmover = SequenceMover( mover1 , mover2 )
	///
	/// Mover    mover1   /first mover to apply with SequenceMover.apply
	/// Mover    mover2   /second mover to apply with SequenceMover.apply
	SequenceMover(MoverOP mover1, MoverOP mover2);

	/// @brief Convenience constructor: initial sequence of 3 movers
	/// seqmover = SequenceMover( mover1 , mover2 , mover3 )
	///
	/// Mover    mover1   /first mover to apply with SequenceMover.apply
	/// Mover    mover2   /second mover to apply with SequenceMover.apply
	/// Mover    mover3   /third mover to apply with SequenceMover.apply
	SequenceMover(MoverOP mover1, MoverOP mover2, MoverOP mover3);

	/// @brief Copy constructor -- performs a deep copy of all contained movers.  Invoked by clone()
	SequenceMover( SequenceMover const & );



	/// @brief deep copy of all contained movers.
	MoverOP clone() const override;

	MoverOP fresh_instance() const override;

	/// @brief MoverStatus of the mover will be evaluated after each mover
	void use_mover_status( bool const flag ){
		use_mover_status_ = flag;
	}

	/// @brief
	bool use_mover_status(){ return use_mover_status_; }

	/// @brief Applies a series of movers sequentially on a Pose
	///
	/// example(s):
	///     seqmover.apply(pose)
	/// See also:
	///     MinMover
	///     RepeatMover
	///     SequenceMover
	///     TrialMover
	void apply( core::pose::Pose & pose ) override;
	std::string get_name() const override;

	static
	std::string
	mover_name();

	void parse_my_tag(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap &
	) override;

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );


private:

	/// @brief use mover status ( default false )
	bool use_mover_status_ = false;

}; // SequenceMover class


/// @brief RandomMover picks a random move and applies it
/// @details If nmoves is greater than 1, it repeats this process
/// nmoves times for each call to apply(). This mover supports
/// weights --- the individual moves are sampled with frequency
/// proportional to their weight given with add_mover( mover, weight );
class RandomMover : public MoverContainer {
public:

	// constructor
	/// APL NOTE: Liz, I'm making up a value of "1.0" here, because you haven't set this value.  Is this a reasonable initial value?
	RandomMover() : MoverContainer(), nmoves_(1), last_proposal_density_ratio_(1.0) {}

	/// @brief Copy constructor.  Performs a deep copy of all contained movers.
	RandomMover( RandomMover const & );

	MoverOP clone() const override;
	MoverOP fresh_instance() const override;

	void apply( core::pose::Pose & pose ) override;

	std::string get_name_individual_mover(core::Size index);

	///@Brief Get the index of the last Mover used by the RandomMover (0 means RandomMover has not been applied yet)
	core::Size
	get_index_of_last_mover_used();

	///@brief Set the repeats for all the movers.  Should be the same order that the movers have been added.
	/// Allows probabilistic repeats
	void
	set_repeats( utility::vector1< core::Size > const & repeats);

	core::Real last_proposal_density_ratio() override;

	//fpd Only making this MoverContainer parsile since the other MoverContainers already have an RS equivalent
	void parse_my_tag(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap &
	) override;

	std::string
	get_name() const override;

	static
	std::string
	mover_name();

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );

private:
	///@Brief Set the index of the last Mover used by the RandomMover (0 means RandomMover has not been applied yet)
	void
	set_index_of_last_mover_used( core::Size index_of_last_mover_used );
	utility::vector1< core::Size > repeats_;

private:
	core::Size nmoves_;
	core::Real last_proposal_density_ratio_; //ek added this member 2/25/10
	core::Size index_of_last_mover_used_ = 0; // Default = 0; But movers_ list is vector0! So 0 actually means the first Mover
}; // RandomMover class


/// @brief CycleMover iterates through its vector of Movers one at a time over many calls to apply().
/// @details Each time CycleMover.apply() is called, it calls apply() on the next Mover in its sequence,
///  until reaching the end of the list and starting over.
///  Useful for things like doing a full repack one out of every eight cycles, and a rotamer trials on the other seven.
class CycleMover : public MoverContainer {
public:

	// constructor
	CycleMover() : MoverContainer(), next_move_(0) {}

	/// @brief Copy constructor.  Performs a deep copy of all contained movers.
	CycleMover( CycleMover const & source );

	/// @brief Clone returns a new instance of CycleMover representing a deep copy of this mover.
	MoverOP clone() const override;

	MoverOP fresh_instance() const override;

	void apply( core::pose::Pose& pose ) override;
	void reset_cycle_index(); //JQX: add reset the cycle mover index next_move_
	std::string get_name() const override;

private:
	core::Size next_move_; //< index into movers_, may need modulo operation first
}; // CycleMover class

/// @brief SwitchMover can hold multiple movers and execute only the requested one
/// @details In terms of RosettaScripts, it allows to have a single script with N-
/// different behaviours that can be called through the parser:script_vars (for example).
/// And it will log in the scores which mover was used.
class SwitchMover : public MoverContainer {
public:

	// constructor
	SwitchMover() : MoverContainer(), user_name_(""), mover_names_(utility::vector0<std::string>()), selected_(""), available_(false) {}

	/// @brief Copy constructor.  Performs a deep copy of all contained movers.
	SwitchMover( SwitchMover const & );

	MoverOP clone() const override;
	MoverOP fresh_instance() const override;

	void apply( core::pose::Pose & pose ) override;

	MoverOP get_selected_mover();
	std::string selected() const { return selected_; };
	void selected( std::string sele ) { selected_ = sele; };

	//fpd Only making this MoverContainer parsile since the other MoverContainers already have an RS equivalent
	void parse_my_tag(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap &
	) override;

	std::string
	get_name() const override;

	static
	std::string
	mover_name();

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );


private:
	std::string user_name_;
	utility::vector0<std::string> mover_names_;
	std::string selected_;
	bool available_;
}; // SwitchMover class

} // moves
} // protocols

#endif //INCLUDED_protocols_moves_MoverContainer_HH
