/*
 * Copyright (C) 2019 - 2020 by the emerging fields initiative 'Novel Biopolymer
 * Hydrogels for Understanding Complex Soft Tissue Biomechanics' of the FAU
 *
 * This file is part of the EFI library.
 *
 * The EFI library is free software; you can use it, redistribute
 * it, and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Author: Stefan Kaessmair
 */

#ifndef SRC_EFI_INCLUDE_EFI_CONSTITUTIVE_OGDEN_H_
#define SRC_EFI_INCLUDE_EFI_CONSTITUTIVE_OGDEN_H_

// stl headers
#include <string>
#include <vector>
#include <iostream>

// deal.II headers
#include <deal.II/base/parameter_handler.h>
#include <deal.II/physics/elasticity/standard_tensors.h>

// efi headers
#include <efi/constitutive/constitutive_base.h>
#include <efi/base/factory_tools.h>
#include <efi/base/logstream.h>
#include <efi/base/automatic_differentiation.h>
#include <efi/base/postprocessor.h>
#include <efi/worker/scratch_data.h>

namespace efi {


/// Ogden material model. For given deformations, it computes the
/// Piola-Kirchoff stresses and its tangent with respect to the
/// right Cauchy-Green deformation tensor.
/// @author Stefan Kaessmair
template <int dim>
class Ogden : public ConstitutiveBase<dim>
{
public:

    using scalar_type = typename ConstitutiveBase<dim>::scalar_type;

    /// Constructor.
    /// @param[in] subsection_name The name of the subsection in the parameter
    /// file which defines the parameters for the constructed instance
    /// of this class.
    /// @param[in] unprocessed_input The unprocessed parts of the parameter
    /// file is everything between the begin of the corresponding subsection
    /// of the to-be-constructed Ogden constitutive model and its end.
    Ogden (const std::string &subsection_name,
           const std::string &unprocessed_input);

    /// Default destructor.
    ~Ogden () = default;

    /// Declare parameters to the given parameter handler.
    /// @param prm The parameter handler for which we want to declare
    /// the parameters.
    void
    declare_parameters (dealii::ParameterHandler &prm) final;

    /// Parse the parameters stored int the given parameter handler.
    /// @param prm The parameter handler whose parameters we want to
    /// parse.
    void
    parse_parameters (dealii::ParameterHandler &prm) final;

    /// This function computes the constitutive response and writes
    /// all fields to the given scratch_data object.
    /// @param[in,out] scratch_data the data object from which we get
    /// the all required data (displacement, etc.) and to which we write
    /// the constitutive response of the @p Ogden material.
    void
    evaluate (ScratchData<dim> &scratch_data) const final;

    /// Return all <tt>dealii::UpdateFlags</tt> update flags required
    /// by this object.
    dealii::UpdateFlags
    get_needed_update_flags () const final;

    /// Return the post-processor information like number of components,
    /// names of the single components etc. we want to stream to output
    /// as vector of @p PostProcessorInfo objects.
    std::vector<DataInterpretation>
    get_data_interpretation () const final;

    /// This function computes the post-processor information we want
    /// to write to the output files e.g. for paraview. Here, the
    /// displacements and the Piola stress tensor are computed.
    /// @param[in] input_data It provides all quantities necessary for the
    /// post processing.
    /// @param[out] computed_quantities The displacement and the Piola stress
    /// tensor are written to this vector. The <tt>std::vector<\tt> has
    /// as many entries as we have evaluation points, the internal <tt>
    /// dealii::Vector<double><\tt> stores the individual components of
    /// the displacement and the Piola stress tensor.
    /// @param[in] dummy Dummy variable
    void
    evaluate_vector_field (const dealii::DataPostprocessorInputs::Vector<dim> &input_data,
                           std::vector<dealii::Vector<double>> &computed_quantities,
                           const dealii::GeneralDataStorage* dummy = nullptr) const final;

    /// Compute the first derivatives of the free energy with to the
    /// principal stretches. The label principal stress might appear
    /// a bit sloppy, but the derivative of the free energy with respect
    /// to a principal stretch yields a principal-stress-like
    /// quantity. The principal stresses computed separately for the
    /// volumetric and the isochoric part of the free energy.
    /// @param[in] lambda The principal stretches.
    /// @param[out] tangent_iso The isochoric principal Kirchoff stresses.
    /// @param[out] tangent_vol The volumetric principal Kirchoff stresses.
    void
    compute_principal_stresses (const std::array<double,dim> &lambda,
                                dealii::Tensor<1,dim,double> &principal_stress_iso,
                                dealii::Tensor<1,dim,double> &principal_stress_vol) const final;

    /// Compute the second derivatives of the free energy with to the
    /// principal stretches. The label principal stress tangents might
    /// appear a bit sloppy, but the derivative of the free energy with
    /// respect to a principal stretch yields a principal-stress-like
    /// quantity.
    /// @param[in] lambda The principal stretches.
    /// @param[out] tangent_iso The derivative of the isochoric principal
    /// Kirchoff stresses with respect to the principal stretches.
    /// @param[out] tangent_vol The derivative of the volumetric principal
    /// Kirchoff stresses with respect to the principal stretches.
    void
    compute_principal_stress_tangents (const std::array<double,dim> &lambda,
                                       dealii::SymmetricTensor<2,dim,double> &principal_stress_tangent_iso,
                                       dealii::SymmetricTensor<2,dim,double> &principal_stress_tangent_vol) const final;

private:

    /// Tension compression anisotropy parameter.
    std::vector<scalar_type> alpha;

    /// Ogden material parameter
    std::vector<scalar_type> mu;

    /// Bulk modulus.
    scalar_type kappa;

    /// Empirical coefficient.
    scalar_type beta;

    /// Number of terms of the Ogden model
    unsigned int n_terms;
};



//----------------------- INLINE AND TEMPLATE FUNCTIONS ----------------------//



template<int dim>
inline
Ogden<dim>::
Ogden(const std::string &subsection_name,
      const std::string &unprocessed_input)
:
    ConstitutiveBase<dim>(subsection_name,unprocessed_input),
    kappa(1.),
    beta(1.),
    n_terms(1)
{
    efilog(Verbosity::verbose) << "New Ogden constitutive model created ("
                               << subsection_name
                               << ")."<< std::endl;
}



template<int dim>
inline
void
Ogden<dim>::
declare_parameters (dealii::ParameterHandler &prm)
{
    using namespace dealii;

    prm.declare_entry ("alpha", "-20",
            Patterns::List(Patterns::Double(), 1,
                    std::numeric_limits<unsigned int>::max(), ","));
    prm.declare_entry ("mu", "0.7e3",
            Patterns::List(Patterns::Double(), 1,
                    std::numeric_limits<unsigned int>::max(), ","));

    prm.declare_entry ("bulk modulus", "3.e3", Patterns::Double());
    prm.declare_entry ("empirical coefficient", "-2.", Patterns::Double());

    efilog(Verbosity::verbose) << "Ogden constitutive model finished declaring "
                                  "parameters."
                               << std::endl;
}



template<int dim>
inline
void
Ogden<dim>::
parse_parameters (dealii::ParameterHandler &prm)
{
    using namespace dealii;

    this->alpha = Utilities::string_to_double (
            Utilities::split_string_list(prm.get ("alpha"), ','));
    this->mu    = Utilities::string_to_double (
            Utilities::split_string_list(prm.get ("mu"), ','));

    this->kappa = prm.get_double ("bulk modulus");
    this->beta  = prm.get_double ("empirical coefficient");

    // Check if we have as many parameters alpha as we have parameters mu.
    AssertThrow(this->alpha.size()==this->mu.size(),
        dealii::ExcDimensionMismatch(this->alpha.size(),this->mu.size()));

    this->n_terms = this->alpha.size();

    efilog(Verbosity::verbose) << "Ogden constitutive model finished parsing "
                                  "parameters."
                               << std::endl;
}



template<int dim>
inline
dealii::UpdateFlags
Ogden<dim>::
get_needed_update_flags () const
{
    return dealii::update_gradients;
}


}// namespace efi

#endif /* SRC_EFI_INCLUDE_EFI_CONSTITUTIVE_OGDEN_H_ */
