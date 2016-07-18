#include "RepresentationCreator.h"
#include "objs/Neuron.h"
#include "reps/NeuronRep.h"
#include "reps/ColumnRep.h"
#include "mappers/ColorMapper.h"
#include "mappers/VariableMapper.h"

namespace neuroscheme
{

  void RepresentationCreator::create(
    const shift::Objects& objects,
    shift::Representations& representations )
  {

    auto _greenMapper = new DiscreteColorMapper( );
    auto _redMapper = new DiscreteColorMapper( );

    _greenMapper->push_back( Color( 0.62 * 255, 1.00 * 255, 0.75 * 255 ));
    _greenMapper->push_back( Color( 0.55 * 255, 0.88 * 255, 0.65 * 255 ));
    _greenMapper->push_back( Color( 0.46 * 255, 0.75 * 255, 0.56 * 255 ));
    _greenMapper->push_back( Color( 0.40 * 255, 0.63 * 255, 0.47 * 255 ));
    _greenMapper->push_back( Color( 0.31 * 255, 0.51 * 255, 0.37 * 255 ));
    _greenMapper->push_back( Color( 0.22 * 255, 0.38 * 255, 0.27 * 255 ));

    _redMapper->push_back( Color( 1.00 * 255, 0.62 * 255, 0.75 * 255 ));
    _redMapper->push_back( Color( 0.88 * 255, 0.55 * 255, 0.65 * 255 ));
    _redMapper->push_back( Color( 0.75 * 255, 0.46 * 255, 0.56 * 255 ));
    _redMapper->push_back( Color( 0.63 * 255, 0.40 * 255, 0.47 * 255 ));
    _redMapper->push_back( Color( 0.51 * 255, 0.31 * 255, 0.37 * 255 ));
    _redMapper->push_back( Color( 0.38 * 255, 0.22 * 255, 0.27 * 255 ));

    _greenMapper->max( ) = 100.0f;
    _redMapper->max( ) = 100.0f;

    const float maxNeuronSomaArea = 100.0f;
    const float maxNeuronDendArea = 100.0f;
    auto _somaAreaToAngle =
      new MapperFloatToFloat( 0, maxNeuronSomaArea, 0.0f, -360.0f);
    auto _dendAreaToAngle =
      new MapperFloatToFloat( 0, maxNeuronDendArea, 0.0f, -360.0f );

    const float maxNeurons = 100.0f;
    auto _neuronsToPercentage =
      new MapperFloatToFloat( 0, maxNeurons, 0.0f, 1.0f );


    for ( const auto obj : objects )
    {
      if ( dynamic_cast< Neuron* >( obj ))
      {
        auto neuron = dynamic_cast< Neuron* >( obj );
        auto neuronRep = new NeuronRep( );

        switch ( neuron->getProperty( "morphoType" ).
                 value< Neuron::TMorphologicalType >( ))
        {
        case Neuron::UNDEFINED_FUNCTIONAL_TYPE:
          neuronRep->setProperty( "symbol", NeuronRep::NO_SYMBOL );
          break;
        case Neuron::INTERNEURON:
          neuronRep->setProperty( "symbol", NeuronRep::TRIANGLE );
          break;
        case Neuron::PYRAMIDAL:
          neuronRep->setProperty( "symbol", NeuronRep::CIRCLE );
          break;
        default:
          break;
        }

        switch (
          neuron->getProperty( "funcType" ).value< Neuron::TFunctionalType >( ))
        {
        case Neuron::UNDEFINED_FUNCTIONAL_TYPE:
          neuronRep->setProperty( "bg", Color( 100, 100, 100 ));
          break;
        case Neuron::INTERNEURON:
          neuronRep->setProperty( "bg", Color( 200, 100, 100 ));
          break;
        case Neuron::PYRAMIDAL:
          neuronRep->setProperty( "bg", Color( 100, 100, 200 ));
          break;
        default:
          break;
        }

        shiftgen::NeuronRep::Rings rings;

        shiftgen::Ring somaRing;
        somaRing.setProperty(
          "angle",
          int(
            roundf(
              _somaAreaToAngle->map(
                neuron->getProperty( "somaVolume" ).value< float >( )))));
        _greenMapper->value( ) =
          neuron->getProperty( "somaArea" ).value< float >( );
        somaRing.setProperty( "color", _greenMapper->map( ));
        rings.push_back( somaRing );

        shiftgen::Ring dendRing;
        dendRing.setProperty(
          "angle",
          int(
            roundf(
              _dendAreaToAngle->map(
                neuron->getProperty( "dendVolume" ).value< float >( )))));
        _redMapper->value( ) =
          neuron->getProperty( "dendArea" ).value< float >( );
        dendRing.setProperty( "color", _redMapper->map( ));
        rings.push_back( dendRing );


        neuronRep->setProperty( "rings", rings );

        representations.push_back( neuronRep );

      } // end if its Neuron objetc

      if ( dynamic_cast< Column* >( obj ))
      {
        auto column = dynamic_cast< Column* >( obj );
        auto columnRep = new ColumnRep( );
        NeuronRep meanNeuronRep;

        meanNeuronRep.setProperty( "symbol", NeuronRep::NO_SYMBOL );
        meanNeuronRep.setProperty( "bg", Color( 100, 100, 100 ));

        shiftgen::ColumnRep::Rings rings;

        shiftgen::Ring somaRing;
        somaRing.setProperty(
          "angle",
          int(
            roundf(
              _somaAreaToAngle->map(
                column->getProperty( "meanSomaVolume" ).
                value< float >( )))));
        _greenMapper->value( ) =
          column->getProperty( "meanSomaArea" ).value< float >( );
        somaRing.setProperty( "color", _greenMapper->map( ));
        rings.push_back( somaRing );

        shiftgen::Ring dendRing;
        dendRing.setProperty(
          "angle",
          int(
            roundf(
              _dendAreaToAngle->map(
                column->getProperty( "meanDendVolume" ).
                value< float >( )))));
        _redMapper->value( ) =
          column->getProperty( "meanDendArea" ).value< float >( );
        dendRing.setProperty( "color", _redMapper->map( ));
        rings.push_back( dendRing );

        meanNeuronRep.registerProperty( "rings", rings );

        columnRep->registerProperty( "meanNeuron", meanNeuronRep );

        shiftgen::ColumnRep::ColumnLayers layersReps;
        shiftgen::LayerRep layerRep;

        (void)_neuronsToPercentage;
          layerRep.setProperty(
            "leftPerc",
            roundf(
              _neuronsToPercentage->map(
                column->getProperty( "Num Pyramidals" ).
                value< float >( ))));
        layerRep.setProperty(
          "rightPerc",
          roundf(
            _neuronsToPercentage->map(
              column->getProperty( "Num Interneurons" ).
              value< float >( ))));
        layersReps.push_back( layerRep );

        for ( unsigned int layer = 1; layer <= 6; ++layer )
        {
          layerRep.setProperty(
            "leftPerc",
              roundf(
                _neuronsToPercentage->map(
                  column->getProperty(
                    std::string( "Num Pyr Layer " ) +
                    std::to_string( layer )).
                  value< float >( ))));
          layerRep.setProperty(
            "rightPerc",
              roundf(
                _neuronsToPercentage->map(
                  column->getProperty(
                    std::string( "Num Inter Layer " ) +
                    std::to_string( layer )).
                  value< float >( ))));
          layersReps.push_back( layerRep );
        }

          // TODO: map percentages
        columnRep->registerProperty( "layers", layersReps );



        representations.push_back( columnRep );
      } // it its Column object
    } // for all objects
  } // create
} // namespace neuroscheme
