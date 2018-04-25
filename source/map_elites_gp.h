#ifndef __MAP_ELITES_GP_H__
#define __MAP_ELITES_GP_H__

#include <set>

#include "Evolve/World.h"
#include "hardware/AvidaGP.h"
#include "config/ArgManager.h"
#include "base/vector.h"
#include "tools/Random.h"
#include "tools/stats.h"
#include "Evolve/World_select.h"

#include "TestcaseSet.h"

EMP_BUILD_CONFIG( MEGPConfig,
  GROUP(DEFAULT, "Default settings for box experiment"),
  VALUE(SEED, int, 55711224, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 100, "Number of organisms in the popoulation."),
  VALUE(UPDATES, uint32_t, 1000, "How many generations should we process?"),
  VALUE(SELECTION, std::string, "MAPELITES", "What selection scheme should we use?"),
  VALUE(MUT_RATE, double, 0.005, "Per-site mutation rate"),
  VALUE(PROBLEM, std::string, "testcases/examples-squares.csv", "Which set of testcases should we use? (or enter 'box' for the box problem"),
  VALUE(N_TEST_CASES, uint32_t, 10, "How many test cases to use"),  
  VALUE(GENOME_SIZE, int, 10, "Length of genome"),
  VALUE(SCOPE_RES, long unsigned int, 16, "Number of bins to make on scope axis"),
  VALUE(ENTROPY_RES, long unsigned int, 10, "Number of bins to make on entropy axis"),
  VALUE(EVAL_TIME, int, 100, "Steps to evaluate for.")
)

class MapElitesGPWorld : public emp::World<emp::AvidaGP> {

public:

    int SEED;
    int GENOME_SIZE;
    int EVAL_TIME;
    long unsigned int SCOPE_RES;
    long unsigned int ENTROPY_RES;
    double MUT_RATE;
    uint32_t POP_SIZE;
    uint32_t UPDATES;
    uint32_t N_TEST_CASES;    
    std::string SELECTION;
    std::string PROBLEM;

    TestcaseSet<int, double> testcases;
    std::function<double(emp::AvidaGP&)> goal_function = [this](emp::AvidaGP org){
        double score = 0;
        emp::Random rand = GetRandom();
        for (int testcase : testcases.GetSubset(N_TEST_CASES, &rand)) {
            org.ResetHardware();
            for (size_t i = 0; i < testcases[testcase].first.size(); i++) {
                org.SetInput(i, testcases[testcase].first[i]);
            }
            org.SetOutput(0,-99999); // Otherwise not outputting anything is a decent strategy
            org.Process(EVAL_TIME);
            int divisor = testcases[testcase].second;
            if (divisor == 0) {
                divisor = 1;
            }   
            double result = 1 / (std::abs(org.GetOutput(0) - testcases[testcase].second)/divisor);
            // emp_assert(std::abs(result) != INFINITY);
            if (result == INFINITY) {
                result = 1000;
            }
            score += result;
        }
        return score;
    };

    MapElitesGPWorld(){;}
    MapElitesGPWorld(emp::Random & rnd) : emp::World<emp::AvidaGP>(rnd) {;}

    std::function<int(emp::AvidaGP &)> scope_count_fun = [this](emp::AvidaGP & val){ 
        std::set<size_t> scopes;
        scopes.insert(val.CurScope());

        for (int i = 0; i < EVAL_TIME; i++) {
            val.Process(1);
            scopes.insert(val.CurScope());
        }
        return scopes.size(); 
    };

    std::function<size_t(emp::AvidaGP::Instruction&)> GetInstID = [](emp::AvidaGP::Instruction & inst){return inst.id;};

    std::function<double(emp::AvidaGP &)> inst_ent_fun = [this](emp::AvidaGP & val){ 
        emp::vector<size_t> inst_list = emp::ApplyFunction(GetInstID, val.GetGenome().sequence);
        return emp::ShannonEntropy(inst_list); 
    };

    void Setup(MEGPConfig & config) {
        SetCache();
        SetMutFun([this](emp::AvidaGP & org, emp::Random & r){
            int count = 0;
            for (int i = 0; i < GENOME_SIZE; ++i) {
                if (r.P(MUT_RATE)) {
                    org.RandomizeInst(i, r);
                    count++;
                }
            }
            return count;
        });
        SetMutateBeforeBirth();
        InitConfigs(config);
        SetupFitnessFile().SetTimingRepeat(10);
        SetupSystematicsFile().SetTimingRepeat(10);
        SetupPopulationFile().SetTimingRepeat(10);
        
        testcases.LoadTestcases(PROBLEM);
        SetFitFun(goal_function);
        AddPhenotype("Num Scopes", scope_count_fun, 0, 16);
        AddPhenotype("Entropy", inst_ent_fun, 0, 6);
        emp::SetMapElites(*this, {SCOPE_RES, ENTROPY_RES});

        InitPop();
    }

    void InitConfigs(MEGPConfig & config) {
        SEED = config.SEED();
        GENOME_SIZE = config.GENOME_SIZE();
        EVAL_TIME = config.EVAL_TIME();
        SCOPE_RES = config.SCOPE_RES();
        ENTROPY_RES = config.ENTROPY_RES();
        MUT_RATE = config.MUT_RATE();
        POP_SIZE = config.POP_SIZE();
        UPDATES = config.UPDATES();
        N_TEST_CASES = config.N_TEST_CASES();    
        SELECTION = config.SELECTION();
        PROBLEM = config.PROBLEM();        
    }

    void InitPop() {
        emp::Random & random = GetRandom();
        for (int i = 0 ; i < 100; i++) {
            emp::AvidaGP cpu;
            cpu.PushRandom(random, GENOME_SIZE);
            Inject(cpu.GetGenome());
        }
    }

    void RunStep() {
        std::cout << update << std::endl;
        emp::RandomSelectSparse(*this, POP_SIZE);
        Update();
    }

    void Run() {
        for (size_t u = 0; u <= UPDATES; u++) {
            RunStep();
        }  
    }

};

#endif
