// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   src/core/energy_methods/SAXSEnergyCreatorFA.hh
/// @brief  Declaration for the class that connects SAXSEnergyCreatorFA with the ScoringManager
/// @author Dominik Gront dgront@chem.uw.edu.pl

#ifndef INCLUDED_core_scoring_saxs_SAXSEnergyCreatorFA_hh
#define INCLUDED_core_scoring_saxs_SAXSEnergyCreatorFA_hh

#include <core/energy_methods/SAXSEnergyCreator.hh>
#include <core/types.hh>



namespace core {
namespace energy_methods {

class SAXSEnergyCreatorFA : public SAXSEnergyCreator {
public:

	SAXSEnergyCreatorFA() {}

	/// @brief Instantiate a new SAXSEnergyFA
	core::scoring::methods::EnergyMethodOP create_energy_method( core::scoring::methods::EnergyMethodOptions const &) const override;

	/// @brief Return the set of score types claimed by the EnergyMethod
	/// this EnergyMethodCreator creates in its create_energy_method() function
	core::scoring::ScoreTypes score_types_for_method() const override;
	virtual
	core::Size version() const;
};

}
}

#endif
