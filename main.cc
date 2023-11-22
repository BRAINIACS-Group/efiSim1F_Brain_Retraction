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

// stl headers
#include <iostream>

// deal.II headers
#include <deal.II/base/mpi.h>
#include <deal.II/base/exceptions.h>

// boost headers
#include <boost/exception/diagnostic_information.hpp>

// efi headers
#include <efi/lab/experiment.h>
#include <efi/factory/registry.h>
#include <efi/base/csv.h>
#include <efi/base/gnuplot_stream.h>
#include <efi/base/global_parameters.h>
#include <efi/grid/obstacle.h>
//#include <efi/base/parameter_modifier.h>


int main (int argc, char *argv[]) {

    using namespace dealii;
    using namespace efi;

    const unsigned int n_threads = Utilities::string_to_int(std::string(argv[1]));

    Utilities::MPI::MPI_InitFinalize mpi_initialization (argc, argv, n_threads);

    constexpr int dim = 3;

    try {

        // print the registry
        Registry::print (efilog(Verbosity::normal));

        std::string parameters (argv[2]);
        init_global_parameters (parameters);

        Experiment<dim> experiment;

        experiment.parse_input (parameters);
        experiment.run ();

    }
    catch (std::exception &exec)
    {
        std::cout << std::flush;

        efilog(Verbosity::normal)
            << "\n\n--------------------------------------------------------\n"
            << "Exception thrown :\n"
            << exec.what() << std::endl
            << "Aborting!\n"
            << "--------------------------------------------------------"
            << std::endl;
        return 1;
    }
    catch (boost::exception &exec)
    {
        std::cout << std::flush;

        efilog(Verbosity::normal)
            << "\n\n--------------------------------------------------------\n"
            << "Exception thrown :\n"
            << boost::diagnostic_information(exec) << std::endl
            << "Aborting!\n"
            << "--------------------------------------------------------"
            << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cout << std::flush;

        efilog(Verbosity::normal)
            << "\n\n--------------------------------------------------------\n"
            << "Unknown exception! \nAborting!\n"
            << "--------------------------------------------------------"
            << std::endl;

        return 1;
    }


    return 0;
}

