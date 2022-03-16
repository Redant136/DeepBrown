#pragma once

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265359
#endif
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#ifdef DEBUG
#include <iostream>

#ifndef print
#define print(x) std::cout << x << std::endl;
#endif

#endif
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#ifdef NeuralNet_NEAT
// includes
#include <vector>
#include <string>
#include <stdexcept>

#endif

#ifdef NeuralNet_GradientDescent
// includes
#include <vector>

#endif

namespace NeuralNet
{
  static void seedRandomNumberGenerator()
  {
    srand((uint32_t)time(0));
  }
  static inline f32 randf()
  {
    return ((f32)rand() / (RAND_MAX));
  }
  static inline f32 randNormal()
  {
    return sqrt(-2 * log(randf())) * cos(2 * M_PI * randf());
  }
  static inline f32 randfNormal()
  {
    return (sqrt(-2 * log(randf())) * cos(2 * M_PI * randf()) + 5) / 10;
  }

#ifdef NeuralNet_NEAT

/**
 * Implementation of NEAT algorithm
 * Override the defines to specify other values
 * define NEAT_NET_VectorNeuralNet if you want neural net to use vectors instead of arrays
 * array neural net allocates all necessary mem at launch and crashes if using too much
 *  can throw stack overflow as it can reach max mem at launch
 * vector neural net uses a vector and allocates mem as time goes on, slower and I found can crash
 **/

#ifndef NEAT_Function
#define NEAT_Function(x) (1 / (1 + exp(-4.9 * x)))
#endif
#ifndef NEAT_maxSpeciesDelta
#define NEAT_maxSpeciesDelta 3.0
#endif
#ifndef NEAT_oddInterspeciesMutation
#define NEAT_oddInterspeciesMutation 0.01
#endif

#ifndef NEAT_c1
#define NEAT_c1 1
#endif
#ifndef NEAT_c2
#define NEAT_c2 1
#endif
#ifndef NEAT_c3
#define NEAT_c3 0.4
#endif

#ifndef NEAT_mutationRange
#define NEAT_mutationRange 10
#endif
#ifndef NEAT_oddMutationWithCrossover
#define NEAT_oddMutationWithCrossover 0.75
#endif
#ifndef NEAT_oddDisabledGeneTransmited
#define NEAT_oddDisabledGeneTransmited 0.75
#endif
#ifndef NEAT_oddAddDisjointExcessGene
#define NEAT_oddAddDisjointExcessGene 0.75
#endif
#ifndef NEAT_oddAddNode
#define NEAT_oddAddNode 0.03
#endif
#ifndef NEAT_oddNewLinkSmallSpecies
#define NEAT_oddNewLinkSmallSpecies 0.05
#endif
#ifndef NEAT_oddNewLinkBigSpecies
#define NEAT_oddNewLinkBigSpecies 0.3
#endif
#ifndef NEAT_oddBias
#define NEAT_oddBias 0.75
#endif
#ifndef NEAT_oddRecursiveLink
#define NEAT_oddRecursiveLink 0.001
#endif
#ifndef NEAT_ratioBigSpecies
#define NEAT_ratioBigSpecies 0.1
#endif
#ifndef NEAT_oddWeightMutate
#define NEAT_oddWeightMutate 0.8
#endif
#ifndef NEAT_oddWeightRandomMutate
#define NEAT_oddWeightRandomMutate 0.1
#endif

  namespace NEAT
  {

#ifdef NEAT_NET_VectorNeuralNet
    template <typename F,size_t InputSize,size_t OutputSize>
    struct NeuralNet final
    {
      struct Node
      {
        F value, raw_value;
        enum
        {
          Node_HID,
          Node_IN,
          Node_OUT,
          Node_BIAS
        } type;
        bool calculated;
      };
      struct NodeLink
      {
        u32 input, output, innov;
        F weight;
        bool enabled, recursive, calculated;
      };
      const static size_t inputSize = InputSize, outputSize = OutputSize;
      std::vector<Node> nodes;
      std::vector<NodeLink> links;
    };

#define NetNodes_length(n) n.nodes.size()
#define NetAddNode(n,node) b.nodes.push_back(nodes);
#define NetAddNodeArg(n,...) n.nodes.push_back({__VA_ARGS__});
#define NetLinks_length(n) n.links.size()
#define NetAddLink(n,link) n.links.push_back(link);
#define NetAddLinkArg(n,...) n.links.push_back({__VA_ARGS__});

#define NEATNet NeuralNet<F,InputSize,OutputSize>
#define NetTemplate typename F,size_t InputSize,size_t OutputSize
#define NetTemplateArg F, InputSize, OutputSize

#else
    template <typename F,size_t InputSize,size_t OutputSize, size_t max_nodes_length=2*(InputSize+OutputSize+1), size_t max_links_length=2*(InputSize*OutputSize)>
    struct NeuralNet final
    {
      struct Node
      {
        F value, raw_value;
        enum
        {
          Node_HID,
          Node_IN,
          Node_OUT,
          Node_BIAS
        } type;
        bool calculated;
      };
      struct NodeLink
      {
        u32 input, output, innov;
        F weight;
        bool enabled, recursive, calculated;
      };
      static const size_t MAX_NODES_LENGTH = max_nodes_length, MAX_LINKS_LENGTH = max_links_length;
      static const u32 inputSize = InputSize, outputSize = OutputSize;
      size_t node_length = 0, link_length = 0;
      Node nodes[max_nodes_length];
      NodeLink links[max_links_length];
    };

#define NetNodes_length(n) n.node_length
#define NetAddNode(n,node) assert(NetNodes_length(n)+1<n.MAX_NODES_LENGTH);n.nodes[NetNodes_length(n)]=node;NetNodes_length(n)++;
#define NetAddNodeArg(n,...) assert(NetNodes_length(n)+1<n.MAX_NODES_LENGTH);n.nodes[NetNodes_length(n)]={__VA_ARGS__};NetNodes_length(n)++;  
#define NetLinks_length(n) n.link_length
#define NetAddLink(n,link) assert(NetLinks_length(n)+1<n.MAX_LINKS_LENGTH);n.links[NetLinks_length(n)]=link;NetLinks_length(n)++;
#define NetAddLinkArg(n,...) assert(NetLinks_length(n)+1<n.MAX_LINKS_LENGTH);n.links[NetLinks_length(n)]={__VA_ARGS__};NetLinks_length(n)++;

#define NEATNet NeuralNet<F,InputSize,OutputSize, max_nodes_length, max_links_length>
#define NetTemplate typename F,size_t InputSize,size_t OutputSize, size_t max_nodes_length, size_t max_links_length
#define NetTemplateArg F, InputSize, OutputSize, max_nodes_length, max_links_length

#endif

#define NetNode NEATNet::Node
#define NetLink NEATNet::NodeLink

#ifdef NEAT_NET_VectorNeuralNet
    template <NetTemplate>
    static NEATNet createNetWithDimensions()
    {
      u32 inputSize=InputSize,outputSize=OutputSize;
      NEATNet net = {std::vector<typename NetNode>(inputSize + outputSize+1), std::vector<typename NetLink>(inputSize * outputSize)};
      for (u32 i = 0; i < inputSize; i++)
      {
        net.nodes[i] = {0.0f, 0.0f, NetNode::Node_IN, false};
      }
      for (u32 i = inputSize; i < inputSize + outputSize; i++)
      {
        net.nodes[i] = {0.0f, 0.0f, NetNode::Node_OUT, false};
      }
      net.nodes[InputSize+OutputSize]={0.0f, 0.0f, NetNode::Node_BIAS, false};
      for (u32 i = 0; i < outputSize; i++)
      {
        for (u32 j = 0; j < inputSize; j++)
        {
          net.links[i * inputSize + j] = {j, i + inputSize, i * inputSize + j, randf(), true, false, false};
        }
      }
      return net;
    }
#else
    template <typename F,size_t InputSize,size_t OutputSize, size_t max_nodes_length=2*(InputSize+OutputSize+1), size_t max_links_length=2*(InputSize*OutputSize)>
    static NEATNet createNetWithDimensions()
    {
      u32 inputSize=InputSize,outputSize=OutputSize;
      NEATNet net = {inputSize + outputSize + 1, inputSize * outputSize};
      for (u32 i = 0; i < inputSize; i++)
      {
        net.nodes[i] = {0.0f, 0.0f, NetNode::Node_IN, false};
      }
      for (u32 i = inputSize; i < inputSize + outputSize; i++)
      {
        net.nodes[i] = {0.0f, 0.0f, NetNode::Node_OUT, false};
      }
      net.nodes[InputSize+OutputSize]={0.0f, 0.0f, NetNode::Node_BIAS, false};
      for (u32 i = 0; i < outputSize; i++)
      {
        for (u32 j = 0; j < inputSize; j++)
        {
          net.links[i * inputSize + j] = {j, i + inputSize, i * inputSize + j, randf(), true, false, false};
        }
      }
      return net;
    }
#endif 

    template <NetTemplate>
    static std::vector<typename NetNode *> getNeuralNetInput(NEATNet &net)
    {
      std::vector<typename NetNode *> r(net.inputSize);
      for (u32 i = 0; i < net.inputSize; i++)
      {
        r[i] = &net.nodes[i];
      }
      return r;
    }
    template <NetTemplate>
    static std::vector<typename NetNode *> getNeuralNetOutput(NEATNet &net)
    {
      std::vector<typename NetNode *> r(net.outputSize);
      for (u32 i = 0; i < net.outputSize; i++)
      {
        r[i] = &net.nodes[i + net.inputSize];
      }
      return r;
    }
    template <NetTemplate>
    static void setNetInputValues(NEATNet &net, std::vector<F> values)
    {
      for (u32 i = 0; i < values.size(); i++)
      {
        net.nodes[i].raw_value = values[i];
        net.nodes[i].value = values[i];
      }
    }
    template <NetTemplate>
    static std::vector<F> getNetOutputValues(NEATNet &net)
    {
      std::vector<F> out(net.outputSize);
      for (u32 i = net.inputSize; i < net.inputSize + net.outputSize; i++)
      {
        out[i - net.inputSize] = net.nodes[i].value;
      }
      return out;
    }
    template <NetTemplate>
    static std::string getWeightsPrint(NEATNet &net)
    {
      std::string r = "";
      for (u32 i = 0; i < NetLinks_length(net); i++)
      {
        if (net.links[i].enabled)
        {
          r += std::to_string(net.links[i].weight);
          r += " (";
          r += std::to_string(net.links[i].input);
          r += ",";
          r += std::to_string(net.links[i].output);
          r += ") ";
          r += std::to_string(net.links[i].innov);
          r += " ";
          r += std::to_string(net.links[i].recursive);
          r += "\n";
        }
      }
      return r;
    }

    template <NetTemplate>
    static void resetNeuralNet(NEATNet &net)
    {
      for (u32 i = 0; i < NetNodes_length(net); i++)
      {
        net.nodes[i].value = 0;
        net.nodes[i].raw_value = 0;
      }
    }
    template <NetTemplate>
    static bool isRecuriveLink(NEATNet &net, u32 in, u32 out)
    {
      bool res = false;
      if (in == out)
      {
        return true;
      }
      for (u32 i = 0; i < NetLinks_length(net); i++)
      {
        if (!res && (!net.links[i].recursive) && net.links[i].output == in)
        {
          if (net.links[i].input == out)
          {
            return true;
          }
          else
          {
            res = isRecuriveLink(net, in, net.links[i].input);
          }
        }
        if (res)
        {
          return true;
        }
      }
      return res;
    }
    template <NetTemplate>
    static u32 getInnovFromHistory(u32 input, u32 output, std::vector<typename NetLink> *linksHistory)
    {
      u32 innov = -1;
      for (u32 j = 0; j < linksHistory->size(); j++)
      {
        if ((*linksHistory)[j].input == input &&
            (*linksHistory)[j].output == output)
        {
          innov = (*linksHistory)[j].innov;
        }
      }
      if (innov == -1)
      {
        innov = linksHistory->size() + 1;
        linksHistory->push_back({input, output, innov, 0.0, false, false, false});
      }
      return innov;
    }
    template <NetTemplate>
    void calculateNeuralNetNode(NEATNet &net, u32 node)
    {
      for (u32 i = 0; i < NetLinks_length(net); i++)
      {
        if (!net.links[i].calculated && !net.links[i].recursive && net.links[i].output == node)
        {
          if (!net.nodes[net.links[i].input].calculated)
          {
            calculateNeuralNetNode(net, net.links[i].input);
          }
          if (net.nodes[net.links[i].input].value * net.links[i].weight != 0)
          {
          }
          net.nodes[net.links[i].output].raw_value += net.nodes[net.links[i].input].value * net.links[i].weight;
          net.links[i].calculated = true;
        }
      }
      net.nodes[node].value = NEAT_Function(net.nodes[node].raw_value);
      net.nodes[node].calculated = true;
    }
    template <NetTemplate>
    static void calculateNeuralNet(NEATNet &net)
    {
      for (u32 i = 0; i < NetLinks_length(net); i++)
      {
        net.links[i].calculated = false;
      }
      std::vector<F> newValues = std::vector<F>(NetNodes_length(net));
      for (u32 i = 0; i < NetNodes_length(net); i++)
      {
        if (net.nodes[i].type == NetNode::Node_IN || net.nodes[i].type == NetNode::Node_BIAS)
        {
          newValues[i] = net.nodes[i].value;
          net.nodes[i].calculated = true;
        }
        else
        {
          net.nodes[i].calculated = false;
        }
      }
      for (u32 i = 0; i < NetLinks_length(net); i++)
      {
        if (net.links[i].recursive)
        {
          newValues[net.links[i].output] += net.nodes[net.links[i].input].value;
          net.links[i].calculated = true;
        }
      }
      for (u32 i = 0; i < NetNodes_length(net); i++)
      {
        net.nodes[i].raw_value = newValues[i];
        net.nodes[i].value = newValues[i];
      }
      for (u32 i = net.inputSize; i < net.inputSize + net.outputSize; i++)
      {
        calculateNeuralNetNode(net, i);
      }
    }

    template <NetTemplate>
    static F calculateNetDelta(NEATNet &n1, NEATNet &n2, const F maxDelta)
    {
      u32 E = 0;
      u32 D = 0;
      F W = 0;
      u32 N = NetLinks_length(n1);
      if (NetLinks_length(n2) > N)
      {
        N = NetLinks_length(n2);
      }

      u32 mostRecentInnov1 = n1.links[n1.inputSize * n1.outputSize - 1].innov;
      u32 mostRecentInnov2 = n2.links[n2.inputSize * n2.outputSize - 1].innov;
      for (u32 i = std::min(n1.inputSize * n1.outputSize, n2.inputSize * n2.outputSize); i < std::max(NetLinks_length(n1), NetLinks_length(n2)); i++)
      {
        if (i < NetLinks_length(n1))
        {
          mostRecentInnov1 = std::max(mostRecentInnov1, n1.links[i].innov);
        }
        if (i < NetLinks_length(n2))
        {
          mostRecentInnov2 = std::max(mostRecentInnov2, n1.links[i].innov);
        }
      }
      bool n2LinkIncluded[n2.MAX_LINKS_LENGTH] = {false};
      u32 similarGenes = 0;
      for (u32 i = 0; i < std::max(NetLinks_length(n1), NetLinks_length(n2)); i++)
      {
        if ((NEAT_c1 * E / N + NEAT_c2 * D / N + NEAT_c3 * W) > maxDelta)
        {
          return NEAT_c1 * E / N + NEAT_c2 * D / N;
        }
        if (i < NetLinks_length(n1))
        {
          if (n1.links[i].innov > mostRecentInnov2)
          {
            E++;
          }
          else
          {
            D++;
            for (u32 k = 0; k < NetLinks_length(n2); k++)
            {
              u32 j = (k + i) % NetLinks_length(n2);
              if (n1.links[i].innov == n2.links[j].innov)
              {
                W += fabsf(n1.links[i].weight - n2.links[j].weight);
                similarGenes++;
                n2LinkIncluded[j] = true;
                D--;
                break;
              }
            }
          }
        }
        if (i < NetLinks_length(n2))
        {
          if (n2.links[i].innov > mostRecentInnov1)
          {
            E++;
          }
        }
      }
      W /= similarGenes;
      for (u32 i = 0; i < NetLinks_length(n2); i++)
      {
        if (!n2LinkIncluded[i])
        {
          D++;
        }
      }

      if (N == 0)
      {
        throw std::invalid_argument("how is N 0");
      }
      if(isnormal(W)){
        return NEAT_c1 * E / N + NEAT_c2 * D / N + NEAT_c3 * W;
      }else{
        return NEAT_c1 * E / N + NEAT_c2 * D / N;
      }
    }
    template <NetTemplate>
    static NEATNet breedNet(NEATNet &parent1, NEATNet &parent2, F speciesRatio,
                                                                     std::vector<typename NetLink> *linksHistory)
    {
      NEATNet offspring = parent1;
      //structure crossover
      if (randf() < NEAT_oddMutationWithCrossover)
      {
        for (u32 i = 0; i < NetLinks_length(offspring); i++)
        {
          for (u32 j = 0; j < NetLinks_length(parent2); j++)
          {
            if (offspring.links[i].innov == parent2.links[j].innov)
            {
              if (randf() < 0.5)
              {
                offspring.links[i].weight = parent2.links[j].weight;
              }
            }
          }
        }
        //add disjoint/excess link
        if (randf() < NEAT_oddAddDisjointExcessGene)
        {
          if (NetNodes_length(parent2) > NetNodes_length(parent1))
          {
            for (u32 i = NetNodes_length(parent1); i < NetNodes_length(parent2); i++)
            {
              NetAddNodeArg(offspring,0.0, 0.0, parent2.nodes[i].type, false)
            }
          }
          for (u32 i = 0; i < NetLinks_length(parent2); i++)
          {
            bool included = false;
            for (u32 j = 0; j < NetLinks_length(offspring); j++)
            {
              if (parent2.links[i].innov == offspring.links[j].innov)
              {
                included = true;
                break;
              }
            }
            if (!included)
            {
              if (parent2.links[i].recursive ||
                  isRecuriveLink(offspring, parent2.links[i].input, parent2.links[i].output) || offspring.nodes[parent2.links[i].input].type == NetNode::Node_OUT)
              {
                NetAddLinkArg(offspring,parent2.links[i].input, parent2.links[i].output, parent2.links[i].innov, parent2.links[i].weight, true, true, false)
              }
              else
              {
                NetAddLinkArg(offspring,parent2.links[i].input, parent2.links[i].output, parent2.links[i].innov, parent2.links[i].weight, true, parent2.links[i].recursive, false)
              }
            }
          }
        }
        //disabled gene transmited
        for (u32 i = 0; i < NetLinks_length(offspring); i++)
        {
          if (!offspring.links[i].enabled)
          {
            if (!(randf() < NEAT_oddDisabledGeneTransmited))
            {
              offspring.links[i].enabled = true;
            }
          }
        }
      }

      //mutate structure
      //add node
      //condition is only here to remove from program simply
      if (1)
      {
        u32 i = floor(randf() * NetLinks_length(offspring));
        if (randf() <= NEAT_oddAddNode && !offspring.links[i].recursive && offspring.links[i].enabled)
        {
          offspring.links[i].enabled = false;

          NetAddNodeArg(offspring,0.0, 0.0, NetNode::Node_HID, false);

          u32 l1Innov=getInnovFromHistory<NetTemplateArg>(offspring.links[i].input, NetNodes_length(offspring) - 1, linksHistory);
          NetAddLinkArg(offspring,offspring.links[i].input, (u32)(NetNodes_length(offspring) - 1), l1Innov, 1.0, true, false, false);

          u32 l2Innov=getInnovFromHistory<NetTemplateArg>(NetNodes_length(offspring) - 1, offspring.links[i].output, linksHistory);
          NetAddLinkArg(offspring,(u32)(NetNodes_length(offspring) - 1), offspring.links[i].output, l2Innov, offspring.links[i].weight, true, false, false)
        }
      }

      //add link
      if (1)
      {
        bool mutate = false;
        if (speciesRatio <= NEAT_ratioBigSpecies)
        {
          mutate = randf() < NEAT_oddNewLinkSmallSpecies;
        }
        else
        {
          mutate = randf() < NEAT_oddNewLinkBigSpecies;
        }
        if (mutate)
        {
          u32 j = floor(randf() * NetNodes_length(offspring));
          while (offspring.nodes[j].type == NetNode::Node_IN || offspring.nodes[j].type == NetNode::Node_BIAS)
          {
            j = floor(randf() * NetNodes_length(offspring));
          }
          u32 i = floor(randf() * NetNodes_length(offspring));
          bool exists = false;
          for (u32 l = 0; l < NetLinks_length(offspring); l++)
          {
            if (offspring.links[l].input == i && offspring.links[l].output == j)
            {
              exists = true;
            }
          }
          if (!exists)
          {
            if (isRecuriveLink(offspring, i, j) || offspring.nodes[i].type == NetNode::Node_OUT)
            {
              if (randf() < NEAT_oddRecursiveLink)
              {
                u32 lInnov=getInnovFromHistory<NetTemplateArg>(i, j, linksHistory);
                NetAddLinkArg(offspring,i, j, lInnov, NEAT_mutationRange * (randf() * 2 - 1), true, true, false);
              }
            }
            else
            {
              if (offspring.nodes[i].type != NetNode::Node_BIAS || randf() < NEAT_oddBias)
              {
                u32 lInnov=getInnovFromHistory<NetTemplateArg>(i, j, linksHistory);
                NetAddLinkArg(offspring,i, j, lInnov, NEAT_mutationRange * (randf() * 2 - 1), true, false, false);
              }
            }
          }
        }
      }

      //change weights
      for (u32 i = 0; i < NetLinks_length(offspring); i++)
      {
        if (randf() < NEAT_oddWeightMutate)
        {
          if (randf() <= NEAT_oddWeightRandomMutate)
          {
            offspring.links[i].weight = NEAT_mutationRange * (randf() * 2 - 1);
          }
          else
          {
            if (abs(offspring.links[i].weight) > NEAT_mutationRange)
            {
              offspring.links[i].weight += NEAT_mutationRange * (randfNormal() * 2 - 1);
            }
            else
            {
              offspring.links[i].weight += NEAT_mutationRange * (randfNormal() * 2 - 1);
              if (abs(offspring.links[i].weight) > NEAT_mutationRange)
              {
                offspring.links[i].weight = abs(offspring.links[i].weight) / offspring.links[i].weight * NEAT_mutationRange;
              }
            }
          }
        }
      }
      return offspring;
    }

    template <size_t population_size, NetTemplate>
    class Population
    {
    public:
      static const size_t MAX_POP_SIZE = population_size;
      struct Species
      {
        u32 age = 0;
      };
      struct Individual
      {
        NEATNet net;
        Species *species = NULL;
        F fitness = 0;
        u32 age = 0;
      };

    protected:
      size_t species_length = 0;
      Species possibleSpecies[population_size];

      virtual void updatePop()
      {
        std::vector<F> offspringsPerSpecies = std::vector<F>(speciesRep.size());
        { // offspring per species
          for (u32 i = 0; i < speciesRep.size(); i++)
          {
            offspringsPerSpecies[i] = 0;
            for (u32 j = 0; j < population_size; j++)
            {
              if (population[j].species == speciesRep[i].species)
              {
                if (isnormal(population[j].fitness))
                {
                  offspringsPerSpecies[i] += population[j].fitness;
                }
              }
            }
          }
          for (u32 i = 0; i < offspringsPerSpecies.size(); i++)
          {
            if (speciesRep[i].age >= 15)
            {
              offspringsPerSpecies[i] = 0;
            }
          }

          F offspringSum = 0;
          for (u32 i = 0; i < offspringsPerSpecies.size(); i++)
          {
            offspringSum += offspringsPerSpecies[i];
          }
          for (u32 i = 0; i < offspringsPerSpecies.size(); i++)
          {
            if (offspringSum == 0)
            {
              offspringsPerSpecies[i] = (f32)population_size / offspringsPerSpecies.size();
            }
            else
            {
              offspringsPerSpecies[i] = offspringsPerSpecies[i] / offspringSum * population_size;
            }
          }
          u32 floorSum = 0;
          for (u32 i = 0; i < offspringsPerSpecies.size(); i++)
          {
            floorSum += floor(offspringsPerSpecies[i]);
          }

          std::vector<F> max = std::vector<F>(0);
          if (floorSum < population_size)
          {
            max = std::vector<F>(population_size - floorSum);
            for (u32 i = 0; i < max.size(); i++)
            {
              max[i] = -1;
            }
            for (u32 i = 0; i < max.size(); i++)
            {
              for (u32 j = 0; j < offspringsPerSpecies.size(); j++)
              {
                bool included = false;
                for (u32 l = 0; l < max.size(); l++)
                {
                  included = included || max[l] == j;
                }
                if (max[i] == -1 || (!included && floor(offspringsPerSpecies[max[i]]) >= floor(offspringsPerSpecies[j])))
                {
                  if (max[i] == -1)
                  {
                    max[i] = j;
                  }
                  else if (offspringsPerSpecies[max[i]] < offspringsPerSpecies[j])
                  {
                    max[i] = j;
                  }
                }
              }
            }
          }
          for (u32 i = 0; i < offspringsPerSpecies.size(); i++)
          {
            offspringsPerSpecies[i] = floor(offspringsPerSpecies[i]);
          }
          for (u32 i = 0; i < max.size(); i++)
          {
            offspringsPerSpecies[max[i]] += 1;
          }
        }

        size_t nextGen_length = 0;
        //add champion of each species with more than 5 ind
        for (u32 i = 0; i < offspringsPerSpecies.size(); i++)
        {
          if (offspringsPerSpecies[i] > 5)
          {
            u32 maxFitIndex = -1;
            F speciesMaxFitness = 0;
            for (u32 j = 0; j < population_size; j++)
            {
              if (population[j].species == speciesRep[i].species)
              {
                if (population[j].fitness > speciesMaxFitness)
                {
                  speciesMaxFitness = population[j].fitness;
                  maxFitIndex = j;
                }
              }
            }
            if (maxFitIndex != -1)
            {
              offspringsPerSpecies[i]--;
              nextGen[nextGen_length] = {
                  population[maxFitIndex].net,
                  population[maxFitIndex].species,
                  0,
                  population[maxFitIndex].age};
              nextGen_length++;
              nextGen[nextGen_length - 1].age += 1;
              speciesRep[i].species->age = nextGen[nextGen_length - 1].age;
            }
          }
        }
        //breeding
        for (u32 i = 0; i < offspringsPerSpecies.size(); i++)
        {
          F speciesFit = 0;
          for (u32 j = 0; j < population_size; j++)
          {
            if (population[j].species == speciesRep[i].species)
            {
              speciesFit += population[j].fitness;
            }
          }
          for (u32 l = 0; l < offspringsPerSpecies[i]; l++)
          {
            u32 parent1 = 0;
            f32 randIndex = randf();
            F currentFitIndex = 0;
            for (u32 j = 0; j < population_size; j++)
            {
              if (population[j].species == speciesRep[i].species)
              {
                currentFitIndex += population[j].fitness;
                if (currentFitIndex / speciesFit > randIndex)
                {
                  parent1 = j;
                  break;
                }
              }
            }
            u32 parent2 = parent1;

            const bool interSpecies = randf() < NEAT_oddInterspeciesMutation;
            currentFitIndex = 0;
            randIndex = randf();
            F parent2SpeciesFit = 0;
            for (u32 j = 0; j < population_size; j++)
            {
              if (interSpecies || population[j].species == speciesRep[i].species)
              {
                parent2SpeciesFit += population[j].fitness;
                if(!isnormal(parent2SpeciesFit+1)){
                  throw std::invalid_argument("not normal");
                }
              }
            }
            for (u32 j = 0; j < population_size; j++)
            {
              if (interSpecies || population[j].species == speciesRep[i].species)
              {
                currentFitIndex += population[j].fitness;
                if (currentFitIndex / parent2SpeciesFit > randIndex)
                {
                  parent2 = j;
                  break;
                }
              }
            }
            if (parent1 == parent2)
            {
              // std::cout<<"same parent"<<std::endl;
              nextGen[nextGen_length].net = population[parent1].net;
              // parent2=(parent2+1)%population_size;
            }
            else
            {
              nextGen[nextGen_length].net = breedNet(population[parent1].net,
                                                    population[parent2].net,
                                                    offspringsPerSpecies[i] / population_size, &linksHistory);
            }
            nextGen_length++;
          }
        }

        assert(nextGen_length == population_size);
        for (u32 i = 0; i < nextGen_length; i++)
        {
          population[i] = nextGen[i];
        }
      }

    public:
      typedef typename NetNode Node;
      typedef typename NetLink NodeLink;

      F(*deltaCalculator)
      (NEATNet &, NEATNet &, F) = calculateNetDelta;
      NEATNet (*netBreeder)(NEATNet &, NEATNet &, F, std::vector<NodeLink> *) = breedNet;

      u32 number_epochs = 0;
      Individual population[population_size];
      Individual nextGen[population_size];
      std::vector<NodeLink> linksHistory;

      std::vector<Individual> speciesRep;
      Population()
      {
        for (u32 i = 0; i < population_size; i++)
        {
          population[i] = {createNetWithDimensions<NetTemplateArg>(), NULL, 0, 0};
        }
        speciesRep = std::vector<Individual>();
        linksHistory = std::vector<typename NetLink>(population[0].NetLinks_length(net));
        for (u32 i = 0; i < population[0].NetLinks_length(net); i++)
        {
          linksHistory[i] = population[0].net.links[i];
        }
        setSpecies();
      }
      void updateHistory()
      {
        if (linksHistory.size() == 0)
        {
          for (u32 i = 0; i < population[0].NetLinks_length(net); i++)
          {
            linksHistory.push_back(population[0].net.links[i]);
          }
        }
        for (u32 net = 0; net < population_size; net++)
        {
          for (u32 i = 0; i < population[net].NetLinks_length(net); i++)
          {
            bool included = false;
            for (u32 j = 0; j < linksHistory.size(); j++)
            {
              if (linksHistory[j].input == population[net].net.links[i].input && linksHistory[j].output == population[net].net.links[i].output)
              {
                if (linksHistory[j].innov != population[net].net.links[i].innov)
                {
                  throw std::invalid_argument("different innov");
                }
                included = true;
              }
            }
            if (!included)
            {
              linksHistory.push_back(population[net].net.links[i]);
            }
          }
        }
        for (u32 i = 0; i < linksHistory.size(); i++)
        {
          if (linksHistory[i].innov != i)
          {
            throw std::invalid_argument("problem in innov");
          }
        }
      }
      virtual void setSpecies()
      {
        if (speciesRep.size() == 0)
        {
          speciesRep = std::vector<Individual>();
          population[0].species = &possibleSpecies[species_length];
          species_length++;
          speciesRep.push_back(population[0]);
          for (u32 i = 1; i < population_size; i++)
          {
            bool newSpecies = true;
            for(u32 j=0;j<speciesRep.size();j++){
              if (deltaCalculator(population[i].net, speciesRep[j].net, NEAT_maxSpeciesDelta) <= NEAT_maxSpeciesDelta)
              {
                population[i].species = speciesRep[j].species;
                newSpecies = false;
                break;
              }
 
            }
            if (newSpecies)
            {
              assert(species_length + 1 < population_size);
              population[i].species = &possibleSpecies[species_length];
              species_length++;
              speciesRep.push_back(population[i]);
            }
          }
        }
        else
        {
          for (u32 i = 0; i < population_size; i++)
          {
            F distance = NEAT_maxSpeciesDelta;
            bool newSpecies = true;
            for(u32 j=0;j<speciesRep.size();j++){
              F ndist = deltaCalculator(population[i].net, speciesRep[j].net, distance);
              if(!isnormal(ndist+1)){
                std::cout<<deltaCalculator(population[i].net, speciesRep[j].net, distance)<<std::endl;
                std::cout << "a" << std::endl;
                throw std::invalid_argument("not normal float");
              }
              if (ndist < distance)
              {
                distance = ndist;
                population[i].species = speciesRep[j].species;
                newSpecies = false;
              }
            }
            if (newSpecies)
            {
              population[i].species = &possibleSpecies[species_length];
              species_length++;
              speciesRep.push_back(population[i]);
            }
          }
        }
      }
      virtual void epoch()
      {
        number_epochs++;

        // lowerFitnessBySpecies
        for (u32 i = 0; i < population_size; i++)
        {
          u32 speciesSize = 1;
          for (u32 j=0;j<population_size;j++)
          {
            if (i != j) {
            }
            if (population[j].fitness <= 0)
            {
              throw std::invalid_argument("fitness less than or equal to 0");
            }
            if (deltaCalculator(population[i].net, population[j].net, NEAT_maxSpeciesDelta) <= NEAT_maxSpeciesDelta)
            {
              speciesSize++;
            }
            if(i==j&&deltaCalculator(population[i].net, population[j].net, NEAT_maxSpeciesDelta) > NEAT_maxSpeciesDelta){
              throw std::invalid_argument("error in delta calc");
            }
          }
          if(speciesSize==0){
            throw std::invalid_argument("species size 0");
          }
          population[i].fitness = population[i].fitness / speciesSize;
          if(!isnormal(population[i].fitness)){
            throw std::invalid_argument("fitness not normal");
          }
        }

        // for(u32 i=0;i<population_size;i++){
        //   print(population[i].fitness);
        // }

        updatePop();
        setSpecies();
        selectNewSpeciesReps();
      }
      virtual void calculate()
      {
      }
      void selectNewSpeciesReps()
      {
        std::vector<Individual> newReps;
        for (u32 i = 0; i < speciesRep.size(); i++)
        {
          f32 randIndex = randf();
          u32 speciesSize = 0;
          for (u32 j = 0; j < population_size; j++)
          {
            if (population[j].species == speciesRep[i].species)
            {
              speciesSize++;
            }
          }
          u32 speciesIndex = 0;
          for (u32 j = 0; j < population_size; j++)
          {
            if (population[j].species == speciesRep[i].species)
            {
              speciesIndex++;
              if (randIndex < speciesIndex / speciesSize)
              {
                newReps.push_back(population[j]);
                break;
              }
            }
          }
        }
        this->speciesRep = newReps;
      }

      NEATNet getBest()
      {
        u32 index = 0;
        F maxFit = population[0].fitness;
        for (u32 i = 0; i < population_size; i++)
        {
          if (maxFit < population[i].fitness)
          {
            maxFit = population[i].fitness;
            index = i;
          }
        }
        return population[index].net;
      }
    };

#ifdef NEAT_NET_VectorNeuralNet
    template<size_t population_size,typename F=f32,class NN=NeuralNet<F,1,1>>
    static Population<population_size,F,NN::inputSize,NN::outputSize> genPopulationFromNet()
    {
      return Population<population_size,F,NN::inputSize,NN::outputSize>();
    }
#else
    template<size_t population_size,typename F=f32,class NN=NeuralNet<F,1,1>>
    static Population<population_size,F,NN::inputSize,NN::outputSize,NN::MAX_NODES_LENGTH,NN::MAX_LINKS_LENGTH> genPopulationFromNet()
    {
      return Population<population_size,F,NN::inputSize,NN::outputSize,NN::MAX_NODES_LENGTH,NN::MAX_LINKS_LENGTH>();
    }
#endif 


#undef NetNodes_length
#undef NetAddNode
#undef NetAddNodeArg 
#undef NetLinks_length
#undef NetAddLink
#undef NetAddLinkArg

#undef NEATNet
#undef NetTemplate
#undef NetNode
#undef NetLink
  }
#endif

#ifdef NeuralNet_GradientDescent

#ifndef GradientDescent_initRandom
#define GradientDescent_initRandom 1
#endif

#ifndef derivative_h
#define derivative_h 0.00001
#endif

  namespace GradientDescent
  {

    enum NodeType
    {
      input,
      hidden,     // fully connected layer
      activation, // only have one input
      output      // set after initialisation
    };

#define NodeDef(name, elements, ...)              \
  template <typename F, __VA_ARGS__>              \
  struct Node_##name final                        \
  {                                               \
    inline static NodeType type = NodeType::name; \
    F value;                                      \
    elements                                      \
  };
    template <typename F>
    struct Node_input final
    {
      inline static NodeType type = NodeType::input;
      F value;
    };
    NodeDef(activation, u32 inputIndex;, F (*func)(F));
    NodeDef(hidden,
            static const size_t INPUT_SIZE = input_size;
            F weights[input_size];
            F bias;
            u32 inputIndexes[input_size];,
                                         size_t input_size);
#undef NodeDef

    template <typename F, size_t node_length, class Node>
    struct NeuralNetLayer
    {
      static const size_t NODE_LENGTH = node_length;
      Node nodes[node_length];
      NeuralNetLayer()
      {
      }
      template <size_t layer_node_length, class layer_node>
      NeuralNetLayer(const NeuralNetLayer<F, layer_node_length, layer_node> &base)
      {
        if constexpr (node_length > 0 && layer_node_length > 0)
        {
          if constexpr (std::is_same<Node, layer_node>::value)
          {
            for (u32 i = 0; i < (node_length < layer_node_length) ? node_length : layer_node_length; i++)
            {
              this->nodes[i] = base.nodes[i];
            }
          }
          else
          {
            assert(typeid(Node) == typeid(layer_node));
          }
        }
        else
        {
          assert(node_length > 0 && layer_node_length > 0);
        }
      }
    };

    template <typename F, size_t num_layers, class... LayersClasses>
    struct NeuralNet;

    namespace
    {
      namespace NodeFuncs
      {
        template <typename F, size_t node_length, class Node>
        static void checkNodes(NeuralNetLayer<F, node_length, Node> layer, Node_input<F> &n)
        {
        }
        template <typename F, size_t node_length, class Node, F (*func)(F)>
        static void checkNodes(NeuralNetLayer<F, node_length, Node> layer, Node_activation<F, func> &n)
        {
          assert(node_length > n.inputIndex);
        }
        template <typename F, size_t node_length, class Node, size_t input_size>
        static void checkNodes(NeuralNetLayer<F, node_length, Node> layer, Node_hidden<F, input_size> &n)
        {
          assert(node_length == input_size);
          for (u32 i = 0; i < input_size; i++)
          {
            assert(node_length > n.inputIndexes[i]);
          }
        }

        template <typename F>
        static void initNode(u32 index, Node_input<F> &n)
        {
          n.value = 0;
        }
        template <typename F, F (*func)(F)>
        static void initNode(u32 index, Node_activation<F, func> &n)
        {
          n.value = 0;
          n.inputIndex = index;
        }
        template <typename F, size_t input_size>
        static void initNode(u32 index, Node_hidden<F, input_size> &n)
        {
          if constexpr (GradientDescent_initRandom)
          {
            for (u32 i = 0; i < input_size; i++)
            {
              n.weights[i] = randf();
            }
            n.bias = randf();
          }
          else
          {
            for (u32 i = 0; i < input_size; i++)
            {
              n.weights[i] = 0.f;
            }
            n.bias = 0.f;
          }
          n.value = 0;
          for (u32 i = 0; i < input_size; i++)
          {
            n.inputIndexes[i] = i;
          }
        }

        template <typename F, size_t node_length, class Node>
        static void calculateNode(NeuralNetLayer<F, node_length, Node> layer, Node_input<F> &n)
        {
        }
        template <typename F, size_t node_length, class Node, F (*func)(F)>
        static void calculateNode(NeuralNetLayer<F, node_length, Node> layer, Node_activation<F, func> &n)
        {
          n->value = func(layer.nodes[n.inputIndex].value);
        }
        template <typename F, size_t node_length, class Node, size_t input_size>
        static void calculateNode(NeuralNetLayer<F, node_length, Node> layer, Node_hidden<F, input_size> &n)
        {
          for (u32 i = 0; i < input_size; i++)
          {
            n.value += layer.nodes[n.inputIndexes[i]].value * n.weights[i];
          }
          n.value += n.bias;
        }

        template <typename F, size_t node_length, class Node>
        static std::vector<F> getNodePreviousLayerDelta(NeuralNetLayer<F, node_length, Node> layer, Node_input<F> &n, F nodeDelta)
        {
          return std::vector<F>();
        }
        template <typename F, size_t node_length, class Node, F (*func)(F)>
        static std::vector<F> getNodePreviousLayerDelta(NeuralNetLayer<F, node_length, Node> layer, Node_activation<F, func> &n, F nodeDelta)
        {
          std::vector<F> deltas(node_length);
          deltas[n.inputIndex] = (func(nodeDelta + derivative_h) - func(nodeDelta)) / derivative_h;
          return deltas;
        }
        template <typename F, size_t node_length, class Node, size_t input_size>
        static std::vector<F> getNodePreviousLayerDelta(NeuralNetLayer<F, node_length, Node> layer, Node_hidden<F, input_size> &n, F nodeDelta)
        {
          std::vector<F> deltas(node_length);
          for (u32 i = 0; i < node_length; i++)
          {
            deltas[i] = n.weights[i];
          }
          return deltas;
        }

        template <typename F, size_t node_length, class Node>
        static void backpropagateFromDelta(F scale, NeuralNetLayer<F, node_length, Node> layer, Node_input<F> &n, F deltas) {}
        template <typename F, size_t node_length, class Node, F (*func)(F)>
        static void backpropagateFromDelta(F scale, NeuralNetLayer<F, node_length, Node> layer, Node_activation<F, func> &n, F deltas) {}
        template <typename F, size_t input_size, class Node>
        static void backpropagateFromDelta(F scale, NeuralNetLayer<F, input_size, Node> layer, Node_hidden<F, input_size> &n, F delta)
        {
          for (u32 i = 0; i < input_size; i++)
          {
            n.weights[i] -= scale * (delta * layer.nodes[i].value);
          }
        }
      }

      namespace NeuralNetFuncs
      {
        template <u32 index, class T>
        static T getLayerExtension(T currentLayer)
        {
          if constexpr (index == 0)
          {
            return currentLayer;
          }
          else
          {
            assert(0 && "index out of bounds");
          }
        }
        template <u32 index, class T, class... Args>
        static auto getLayerExtension(T currentLayer, Args... args)
        {
          if constexpr (index == 0)
          {
            return currentLayer;
          }
          else
          {
            return getLayerExtension<index - 1, Args...>(args...);
          }
        }

        template <u32 i, u32 to>
        struct static_for
        {
          template <typename F, size_t num_layers, class... LayersClasses>
          void calculate_net(NeuralNet<F, num_layers, LayersClasses...> &net);
          template <typename F, size_t num_layers, class... LayersClasses>
          void checkLayers(NeuralNet<F, num_layers, LayersClasses...> &net);
          template <typename F, size_t num_layers, class... LayersClasses>
          void getNodesByType(NeuralNet<F, num_layers, LayersClasses...> &net, std::vector<void *> *storage, NodeType type);
        };
        template <u32 to>
        struct static_for<to, to>
        {
          template <typename F, size_t num_layers, class... LayersClasses>
          void calculate_net(NeuralNet<F, num_layers, LayersClasses...> &net) {}
          template <typename F, size_t num_layers, class... LayersClasses>
          void checkLayers(NeuralNet<F, num_layers, LayersClasses...> &net){};
          template <typename F, size_t num_layers, class... LayersClasses>
          void getNodesByType(NeuralNet<F, num_layers, LayersClasses...> &net, std::vector<void *> *storage, NodeType type) {}
        };
      }
    }

    template <typename F, size_t num_layers, class... LayersClasses>
    struct NeuralNet
    {
      static const size_t NUMBER_LAYERS = num_layers;
      void *layers[num_layers];

      NeuralNet(bool initialiseNodes_Weights_In, LayersClasses &...layers)
      {
        initLayers(0, &layers...);
        if (initialiseNodes_Weights_In)
        {
          initNodes<0, LayersClasses...>(&layers...);
        }
        checkNodes<0, LayersClasses...>(layers...);
      }
      void check()
      {
        NeuralNetFuncs::static_for<0, num_layers>().checkLayers(*this);
      }
      template <u32 index>
      auto getLayer()
      {
        assert(index < num_layers);
        return NeuralNetFuncs::getLayerExtension<index, LayersClasses...>((*(LayersClasses *)layers[index])...);
      }
      template <u32 index>
      auto getLayerPointer()
      {
        assert(index < num_layers);
        return NeuralNetFuncs::getLayerExtension<index, LayersClasses *...>(((LayersClasses *)layers[index])...);
      }
      void calculate()
      {
        NeuralNetFuncs::static_for<1, num_layers>().calculate_net(*this);
      }

      void setInput(std::vector<F> values) // only used if input layer is first, this should normally be the case
      {
        NeuralNetLayer input = getLayer<0>();
        assert(input.nodes[0].type == NodeType::input);
        assert(input.NODES_LENGTH == values.size());
        for (u32 i = 0; i < input.NODES_LENGTH; i++)
        {
          input.nodes[i].value = values[i];
        }
      }
      std::vector<F> getOutput()
      {
        NeuralNetLayer output = getLayer<num_layers - 1>();
        std::vector<F> values(output.NODES_LENGTH);
        assert(output.nodes[0].type == NodeType::output);
        for (u32 i = 0; i < output.NODES_LENGTH; i++)
        {
          values[i] = output.nodes[i].value;
        }
        return values;
      }
      std::vector<Node_input<F> *> getInputNodes()
      {
        std::vector<void *> nodes;
        NeuralNetFuncs::static_for<0, num_layers>().getNodesByType(*this, &nodes, NodeType::input);
        return std::vector<Node_input<F> *>(nodes.begin(), nodes.end());
      }
      template <typename Node>
      std::vector<Node *> getNodesByType(NodeType type)
      {
        std::vector<void *> nodes;
        NeuralNetFuncs::static_for<0, num_layers>().getNodesByType(*this, &nodes, type);
        return std::vector<Node *>(nodes.begin(), nodes.end());
      }

      void gradientDescentFromOut(F scale, std::vector<F> expectedOut, F (*cost)(F, F))
      {
        std::vector<F> outValues = getOutput();
        std::vector<F> deltas(expectedOut.size());
        for (u32 i = 0; i < expectedOut.size(); i++)
        {
          deltas[i] = (cost(expectedOut[i] + derivative_h, outValues[i]) - cost(expectedOut[i], outValues[i])) / derivative_h;
        }
        gradDescent<num_layers - 1>(scale, deltas);
      }
      template <u32 currentLayerIndex>
      void gradDescent(F scale, std::vector<F> layerDelta)
      {
        NeuralNetLayer layer = getLayer<currentLayerIndex>();
        assert(layer.NODE_LENGTH == layerDelta.size());
        NeuralNetLayer previousLayer = getLayer<currentLayerIndex - 1>();
        std::vector<F> previousDelta(previousLayer.NODES_LENGTH);
        for (u32 i = 0; i < layer.NODE_LENGTH; i++)
        {
          std::vector<F> nodeDelta = NodeFuncs::getNodePreviousLayerDelta(previousLayer, layer.nodes[i], layerDelta[i]);
          for (u32 j = 0; j < previousLayer.NODE_LENTH; j++)
          {
            previousDelta[j] += nodeDelta[j];
          }
        }
        for (u32 i = 0; i < layer.NODE_LENGTH; i++)
        {
          NodeFuncs::backpropagateFromDelta(scale, previousLayer, layer.nodes[i], layerDelta[i]);
        }
        // F layerDeltas[layer.NODE_LENGTH];
        // for (u32 i = 0; i < nextLayer.NODE_LENGTH; i++)
        // {
        //   F *nodeDeltas = NodeFuncs::getNodePreviousLayerDelta(layer, nextLayer.nodes[i]);
        //   for (u32 j = 0; j < layer.NODE_LENGTH; j++)
        //   {
        //     layerDeltas[j] += nodeDeltas[j];
        //   }
        // }
        // for (u32 i = 0; i < nextLayer.NODE_LENGTH; i++)
        // {
        // }
        // std::vector<F> nodeDeltas(layer.NODE_LENGTH);
        // for (u32 i = 0; i < NetNodes_length(net); i++)
        // {
        //   nodeDeltas[i] = getNodeDelta(i, net, lastLayerDelta);
        // }
        // for (int i = 0; i < NetLinks_length(net); i++)
        // {
        //   net.links[i].weight = net.links[i].weight - scale * (nodeDeltas[net.links[i].output] *
        //                                                        4.9 * exp(-4.9 * net.nodes[net.links[i].input].rawValue)
        //                                                       / pow((1 + exp(-4.9 * net.nodes[net.links[i].input].rawValue)), 2) *
        //                                                        net.nodes[net.links[i].input].value);
        // }
      }

    private:
      template <class T>
      void initLayers(u32 index, T *layer)
      {
        assert(index < num_layers);
        layers[index] = layer;
      }
      template <class T, class... Args>
      void initLayers(u32 index, T *layer, Args *...args)
      {
        assert(index < num_layers);
        layers[index] = layer;
        initLayers(index + 1, args...);
      }

      template <u32 index, class T>
      void initNodes(T *layer)
      {
        for (u32 i = 0; i < layer->NODE_LENGTH; i++)
        {
          NodeFuncs::initNode(i, layer->nodes[i]);
        }
      }
      template <u32 index, class T, class... Args>
      void initNodes(T *layer, Args *...args)
      {
        for (u32 i = 0; i < layer->NODE_LENGTH; i++)
        {
          NodeFuncs::initNode(i, layer->nodes[i]);
        }
        initNodes<index + 1, Args...>(args...);
      }

      template <u32 index, class T>
      void checkNodes(T layer)
      {
        if constexpr (index > 0)
        {
          NeuralNetLayer lastLayer = getLayer<index - 1>();
          for (u32 j = 0; j < layer.NODE_LENGTH; j++)
          {
            NodeFuncs::checkNodes(lastLayer, layer.nodes[j]);
          }
        }
      }
      template <u32 index, class T, class... Args>
      void checkNodes(T layer, Args... args)
      {
        if constexpr (index > 0)
        {
          NeuralNetLayer lastLayer = getLayer<index - 1>();
          for (u32 j = 0; j < layer.NODE_LENGTH; j++)
          {
            NodeFuncs::checkNodes(lastLayer, layer.nodes[j]);
          }
        }
        else
        {
          NeuralNetLayer lastLayer = getLayer<index>();
          for (u32 j = 0; j < layer.NODE_LENGTH; j++)
          {
            NodeFuncs::checkNodes(lastLayer, layer.nodes[j]);
          }
        }
        checkNodes<index + 1, Args...>(args...);
      }
    };

    namespace
    {
      template <u32 i, u32 to>
      template <typename F, size_t num_layers, class... LayersClasses>
      void NeuralNetFuncs::static_for<i, to>::calculate_net(NeuralNet<F, num_layers, LayersClasses...> &net)
      {
        NeuralNetLayer layer = net.template getLayer<i>();
        NeuralNetLayer lastLayer = net.template getLayer<i - 1>();
        for (u32 j = 0; j < layer.NODE_LENGTH; j++)
        {
          NodeFuncs::calculateNode(lastLayer, layer.nodes[j]);
        }
        static_for<i + 1, to>().calculate_net(net);
      }
      template <u32 i, u32 to>
      template <typename F, size_t num_layers, class... LayersClasses>
      void NeuralNetFuncs::static_for<i, to>::checkLayers(NeuralNet<F, num_layers, LayersClasses...> &net)
      {
        NeuralNetLayer layer = net.template getLayer<i>();
        if constexpr (i > 0)
        {
          NeuralNetLayer lastLayer = net.template getLayer<i - 1>();
          for (u32 j = 0; j < layer.NODE_LENGTH; j++)
          {
            NodeFuncs::checkNodes(lastLayer, layer.nodes[j]);
          }
        }
        else
        {
          NeuralNetLayer lastLayer = net.template getLayer<i>();
          for (u32 j = 0; j < layer.NODE_LENGTH; j++)
          {
            NodeFuncs::checkNodes(lastLayer, layer.nodes[j]);
          }
        }
        static_for<i + 1, to>().checkLayers(net);
      }

      template <u32 i, u32 to>
      template <typename F, size_t num_layers, class... LayersClasses>
      void NeuralNetFuncs::static_for<i, to>::getNodesByType(NeuralNet<F, num_layers, LayersClasses...> &net, std::vector<void *> *storage, NodeType type)
      {
        NeuralNetLayer layer = net.template getLayer<i>();
        assert(layer.NODES_LENGTH > 0);
        if (layer.nodes[0].type == type)
        {
          for (u32 j = 0; j < layer.NODES_LENGTH; j++)
          {
            (*storage).push_back(&layer.nodes[j]);
          }
        }
        static_for<i + 1, to>().getNodesByType(net, storage, type);
      }
    }

    template <typename F>
    struct NeuralNetBuilder
    {
      template <class... LayersClasses>
      NeuralNet<F, sizeof...(LayersClasses), LayersClasses...> buildNet(bool initialiseNodes_Weights_Input, LayersClasses... layers)
      {
        return NeuralNet<F, sizeof...(LayersClasses), LayersClasses...>(initialiseNodes_Weights_Input, layers...);
      }
    };

    // namespace NEATNetGradientDescent
    // {
    //   class GradDescent
    //   {
    //   public:
    //     void gradDescent(double scale, NEATLinkedNet net, std::vector<double> lastLayerDelta)
    //     {
    //       std::vector<double> nodeDeltas = std::vector<double>(NetNodes_length(net));
    //       for (int i = 0; i < NetNodes_length(net); i++)
    //       {
    //         nodeDeltas[i] = getNodeDelta(i, net, lastLayerDelta);
    //       }
    //       for (int i = 0; i < NetLinks_length(net); i++)
    //       {
    //         net.links[i].weight = net.links[i].weight - scale * (nodeDeltas[net.links[i].output] *
    //                                                              4.9 * exp(-4.9 * net.nodes[net.links[i].input].rawValue) / pow((1 + exp(-4.9 * net.nodes[net.links[i].input].rawValue)), 2) *
    //                                                              net.nodes[net.links[i].input].value);
    //       }
    //     }
    //     void gradDescent(double scale, NEATLinkedNet net, std::vector<double> expectedOut,
    //                      std::vector<double> (*cost)(std::vector<double> out, std::vector<double> expectedOut))
    //     {
    //       gradDescent(scale, net, cost(net.getOutputValues(), expectedOut));
    //     }
    //   private:
    //     double getNodeDelta(int index, NEAT::NEATLinkedNet net, std::vector<double> lastLayerDelta)
    //     {
    //       if (net.nodes[index].label == "out")
    //       {
    //         int outIndex = 0;
    //         for (int i = 0; i < index; i++)
    //         {
    //           if (net.nodes[i].label == "out")
    //           {
    //             outIndex++;
    //           }
    //         }
    //         return lastLayerDelta[outIndex];
    //       }
    //       double delta = 0;
    //       for (int i = 0; i < NetLinks_length(net); i++)
    //       {
    //         if (net.links[i].input == index)
    //         {
    //           delta += net.links[i].weight * 4.9 * exp(-4.9 * net.nodes[net.links[i].input].rawValue) / pow((1 + exp(-4.9 * net.nodes[net.links[i].input].rawValue)), 2) * getNodeDelta(net.links[i].output, net, lastLayerDelta);
    //         }
    //       }
    //       return delta;
    //     }
    //   };
    // }; // namespace NEATNetGradientDescent
  }
#endif

}