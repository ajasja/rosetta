// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file src/protocols/relax/LocalRelax.hh
/// @brief
/// @details
/// @author


#ifndef INCLUDED_protocols_relax_LocalRelax_fwd_hh
#define INCLUDED_protocols_relax_LocalRelax_fwd_hh


// Utility headers
#include <utility/pointer/access_ptr.hh>
#include <utility/pointer/owning_ptr.hh>


namespace protocols {
namespace relax {

// Forward
class LocalRelax;

// Types
typedef  utility::pointer::shared_ptr< LocalRelax >  LocalRelaxOP;
typedef  utility::pointer::shared_ptr< LocalRelax const >  LocalRelaxCOP;

typedef  utility::pointer::weak_ptr< LocalRelax >  LocalRelaxAP;
typedef  utility::pointer::weak_ptr< LocalRelax const >  LocalRelaxCAP;


} // namespace kinematics
} // namespace core

#endif
