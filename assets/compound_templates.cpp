#include "compound_templates.hpp"
#include "compound.hpp"
#include "colors.hpp"
#include "compound.hpp"
#include <string>
#include <unordered_map>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<i32_t> dis(0, 1'000'000);

std::unordered_map<std::string, Compound> *Compound::templates = &compounds_templates;



// All types a compound can have
enum CompoundType : u32_t {
    NONE = 0x00000001,

    // Custom compound types
    DNA = 0x00000002,
    RNA = 0x00000004,
    PROTEIN = 0x00000008,
    PHOSPHOLIPID = 0x00000010,

    // Add your own types
    // Possible formats:
    // CUSTOM_NAME,
    // CUSTOM_NAME_WITH_VALUE = <custom value in hexadecimal>,
    




    // End of custom compound types section

    ALL = 0x7FFFFFFF, // !RESERVED VALUE. DO NOT MODIFY!
};



// Interaction function names/ids (each interaction function has to have its own id/value)
enum CompoundInteractionType : u32_t {
    DESTROY = 1,
    SYNTH_TRANSCR,
    SYNTH_TRANSLAT,
    DNA_REPL,

    // Add your own function name
    // Format:
    // CUSTOM_NAME,
    




    // End of custom compound types section
};



// Templates of usable compounds
std::unordered_map<std::string, Compound> compounds_templates {

    {"pusher", {
        CompoundType::ALL,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 1.0, WHITE, false},
        },
        {},
        {},
        CompoundType::NONE,
        }
    },

    {"destroyer", {
        CompoundType::NONE,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 1.0, RED, false},
        },
        {
            {CompoundType::DNA, getInteraction(CompoundInteractionType::DESTROY)},
            {CompoundType::RNA, getInteraction(CompoundInteractionType::DESTROY)},
            {CompoundType::PROTEIN, getInteraction(CompoundInteractionType::DESTROY)},
            {CompoundType::PHOSPHOLIPID, getInteraction(CompoundInteractionType::DESTROY)},
        },
        {},
        CompoundType::NONE,
        }
    },

    {"sample-phospholipid", {
        CompoundType::PHOSPHOLIPID,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, YELLOW, true},
            {{0.5, 0.0}, 0.25, CORNFLOWER_BLUE, false}
        },
        {},
        {},
        CompoundType::ALL,
        }
    },

    {"sample-phospholipid-long", {
        CompoundType::PHOSPHOLIPID,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, YELLOW, true},
            {{0.5, 0.0}, 0.25, YELLOW, true},
            {{1.0, 0.0}, 0.25, CORNFLOWER_BLUE, false}
        },
        {},
        {},
        CompoundType::ALL,
        }
    },

    {"sample-dna", {
        CompoundType::DNA,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, WHITE, false},
            {{0.5, 0.0}, 0.25, WHITE, false},
            {{1.0, 0.0}, 0.25, WHITE, false},
            {{1.5, 0.0}, 0.25, WHITE, false},
            {{2.0, 0.0}, 0.25, WHITE, false},
            {{2.5, 0.0}, 0.25, WHITE, false},
            {{3.0, 0.0}, 0.25, WHITE, false},
            {{3.5, 0.0}, 0.25, WHITE, false},
            {{4.0, 0.0}, 0.25, WHITE, false},
            {{4.5, 0.0}, 0.25, WHITE, false},
            {{5.0, 0.0}, 0.25, WHITE, false},
        },
        {},
        {"rna-p1", "rna-p2", "rna-p3"},
        (CompoundType::PHOSPHOLIPID),
        }
    },

    {"sample-rna-p1", {
        CompoundType::RNA,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, MAGENTA, false},
            {{0.5, 0.0}, 0.25, MAGENTA, false},
            {{1.0, 0.0}, 0.25, MAGENTA, false},
            {{1.5, 0.0}, 0.25, MAGENTA, false},
            {{2.0, 0.0}, 0.25, MAGENTA, false},
            {{2.5, 0.0}, 0.25, MAGENTA, false},
            {{3.0, 0.0}, 0.25, MAGENTA, false},
            {{3.5, 0.0}, 0.25, MAGENTA, false},
            {{4.0, 0.0}, 0.25, MAGENTA, false},
            {{4.5, 0.0}, 0.25, MAGENTA, false},
            {{5.0, 0.0}, 0.25, MAGENTA, false},
        },
        {},
        {"protein-1-transcription"},
        (CompoundType::PHOSPHOLIPID),
        1.0,
        }
    },

    {"sample-rna-p2", {
        CompoundType::RNA,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, YELLOW, false},
            {{0.5, 0.0}, 0.25, YELLOW, false},
            {{1.0, 0.0}, 0.25, YELLOW, false},
            {{1.5, 0.0}, 0.25, YELLOW, false},
            {{2.0, 0.0}, 0.25, YELLOW, false},
            {{2.5, 0.0}, 0.25, YELLOW, false},
            {{3.0, 0.0}, 0.25, YELLOW, false},
            {{3.5, 0.0}, 0.25, YELLOW, false},
            {{4.0, 0.0}, 0.25, YELLOW, false},
            {{4.5, 0.0}, 0.25, YELLOW, false},
            {{5.0, 0.0}, 0.25, YELLOW, false},
        },
        {},
        {"protein-2-translation"},
        (CompoundType::PHOSPHOLIPID),
        }
    },

    {"sample-rna-p3", {
        CompoundType::RNA,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, CYAN, false},
            {{0.5, 0.0}, 0.25, CYAN, false},
            {{1.0, 0.0}, 0.25, CYAN, false},
            {{1.5, 0.0}, 0.25, CYAN, false},
            {{2.0, 0.0}, 0.25, CYAN, false},
            {{2.5, 0.0}, 0.25, CYAN, false},
            {{3.0, 0.0}, 0.25, CYAN, false},
            {{3.5, 0.0}, 0.25, CYAN, false},
            {{4.0, 0.0}, 0.25, CYAN, false},
            {{4.5, 0.0}, 0.25, CYAN, false},
            {{5.0, 0.0}, 0.25, CYAN, false},
        },
        {},
        {"protein-3-dna-repl"},
        (CompoundType::PHOSPHOLIPID),
        }
    },

    {"sample-protein-1-transcription", {
        CompoundType::PROTEIN,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, RED, false},
            {{0.5, 0.0}, 0.25, RED, false},
            {{-0.5, 0.0}, 0.25, RED, false},
            {{0.0, 0.5}, 0.25, RED, false},
            {{0.0, -0.5}, 0.25, RED, false},
        },
        {{CompoundType::DNA, getInteraction(CompoundInteractionType::SYNTH_TRANSCR)}},
        {},
        CompoundType::PHOSPHOLIPID,
        }
    },

    {"sample-protein-2-translation", {
        CompoundType::PROTEIN,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, GREEN, false},
            {{0.5, 0.0}, 0.25, GREEN, false},
            {{-0.5, 0.0}, 0.25, GREEN, false},
            {{0.0, 0.5}, 0.25, GREEN, false},
            {{0.0, -0.5}, 0.25, GREEN, false},
        },
        {{CompoundType::RNA, getInteraction(CompoundInteractionType::SYNTH_TRANSLAT)}},
        {},
        CompoundType::PHOSPHOLIPID,
        }
    },

    {"sample-protein-3-dna-repl", {
        CompoundType::PROTEIN,
        {0.0, 0.0},
        {
            {{0.0, 0.0}, 0.25, BLUE, false},
            {{0.5, 0.0}, 0.25, BLUE, false},
            {{-0.5, 0.0}, 0.25, BLUE, false},
            {{0.0, 0.5}, 0.25, BLUE, false},
            {{0.0, -0.5}, 0.25, BLUE, false},
        },
        {{CompoundType::DNA, getInteraction(CompoundInteractionType::DNA_REPL)}},
        {},
        CompoundType::PHOSPHOLIPID,
        }
    },

    //  Add your own compound templates here
    //  Format:
    //
    //  { "compound_name", {
    //      CompoundType,
    //      {position_x, position_y},
    //      {    
    //          List of Particles e.g.
    //          {{relative_position_x, relative_position_y}, radius in nm, COLOR, hydrophobic? <true/false>}
    //          {{relative_position_x, relative_position_y}, radius in nm, {color_r, color_g, color_b}, hydrophobic? <true/false>}
    //          ...
    //      },
    //      {
    //          List of custom functions called when a compound with a specific type is touched
    //          {CompoundType, getInteraction(<the interaction function's id>)}
    //      }
    //
    //      Names of Compounds that are encoded (mainly used by DNA/RNA to specify product of transcription/translation)
    //      {"compound_1_name", "compound_2_name", ...},
    //
    //      Compound types this compound interacts with through hydrophobic/hydrophibic interaction
    //      CompoundType::TYPE_1 || CompoundType::TYPE_2 || ...,
    //      }
    //  },





    // End of custom compound templates section
};



// Interaction functions
std::function<CompoundInteractionType(
Compound*, int,
Compound*, int)> getInteraction(CompoundInteractionType id) {

    u64_t timeStamp = 0;
    bool active = true;
    switch (id) {

    case CompoundInteractionType::DESTROY:
        return [id, timeStamp, active](Compound *_, int, Compound *cmpnd, int) mutable {
            if (!_) return id;

            deleteElems.insert(cmpnd);

            return NO_INTERACTION_TYPE;
        };

    case CompoundInteractionType::SYNTH_TRANSCR:
        return [id, timeStamp, active](Compound *protein, int pProtein, Compound *dna, int pDNA) mutable {
            if (!protein) return id;

            if (timeStamp == 0) {
                if (pDNA == 0) {
                    protein->pos = {dna->pos.X + dna->particles[pDNA].pos.X, dna->pos.Y + dna->particles[pDNA].pos.Y};
                    globalTimer.recordNow();
                    timeStamp = frameCounter;
                }
            }
            else if (timeStamp > 0 && active) {
                if (pDNA == dna->particles.size() - 1) {
                    addElems.push_back(Compound(compounds_templates.at(dna->encodes[dis(gen) % dna->encodes.size()]), protein->pos, {protein->linearVel.Y, -protein->linearVel.X}));
                    globalTimer.deleteEntry(timeStamp);

                    active = false;
                    globalTimer.recordNow();
                    timeStamp = frameCounter;
                } else {
                    f32_t currentX = protein->pos.X + protein->particles[pProtein].pos.X;
                    f32_t currentY = protein->pos.Y + protein->particles[pProtein].pos.Y;
                    Particle &p = dna->particles[pDNA];
                    f64_t theta = dna->angularVel * transcrMultiplier;
                    f64_t s = std::sin(theta);
                    f64_t c = std::cos(theta);
                    f32_t futureX = dna->pos.X + (dna->linearVel.X * transcrMultiplier) + (p.pos.X * c - p.pos.Y * s);
                    f32_t futureY = dna->pos.Y + (dna->linearVel.Y * transcrMultiplier) + (p.pos.X * s + p.pos.Y * c);
                    protein->linearVel = {(futureX - currentX) * transcrMultiplier, (futureY - currentY) * transcrMultiplier};
                }
            }
            else if (globalTimer.Read(timeStamp) > proteinInactivityTime) {
                globalTimer.deleteEntry(timeStamp);

                active = true;
                timeStamp = 0;
            }

            return NO_INTERACTION_TYPE;
        };

    case CompoundInteractionType::SYNTH_TRANSLAT:
        return [id, timeStamp, active](Compound *protein, int pProtein, Compound *rna, int pRNA) mutable {
            if (!protein) return id;

            if (timeStamp == 0) {
                if (pRNA == 0) {
                    protein->pos = {rna->pos.X + rna->particles[pRNA].pos.X, rna->pos.Y + rna->particles[pRNA].pos.Y};
                    globalTimer.recordNow();
                    timeStamp = frameCounter;
                }
            }
            else if (timeStamp > 0 && active) {
                if (pRNA == rna->particles.size() - 1) {
                    addElems.push_back(Compound(compounds_templates.at(rna->encodes[dis(gen) % rna->encodes.size()]), protein->pos, {protein->linearVel.Y, -protein->linearVel.X}));
                    globalTimer.deleteEntry(timeStamp);

                    active = false;
                    globalTimer.recordNow();
                    timeStamp = frameCounter;
                } else {
                    f32_t currentX = protein->pos.X + protein->particles[pProtein].pos.X;
                    f32_t currentY = protein->pos.Y + protein->particles[pProtein].pos.Y;
                    Particle &p = rna->particles[pRNA];
                    f64_t theta = rna->angularVel * translatMultiplier;
                    f64_t s = std::sin(theta);
                    f64_t c = std::cos(theta);
                    f32_t futureX = rna->pos.X + (rna->linearVel.X * translatMultiplier) + (p.pos.X * c - p.pos.Y * s);
                    f32_t futureY = rna->pos.Y + (rna->linearVel.Y * translatMultiplier) + (p.pos.X * s + p.pos.Y * c);
                    protein->linearVel = {(futureX - currentX) * translatMultiplier, (futureY - currentY) * translatMultiplier};
                }
            }
            else if (globalTimer.Read(timeStamp) > proteinInactivityTime) {
                globalTimer.deleteEntry(timeStamp);

                active = true;
                timeStamp = 0;
            }            return NO_INTERACTION_TYPE;
        };

    case CompoundInteractionType::DNA_REPL:
        return [id, timeStamp, active](Compound *protein, int pProtein, Compound *dna, int pDNA) mutable {
            if (!protein) return id;

            if (timeStamp == 0) {
                if (pDNA == 0) {
                    protein->pos = {dna->pos.X + dna->particles[pDNA].pos.X, dna->pos.Y + dna->particles[pDNA].pos.Y};
                    globalTimer.recordNow();
                    timeStamp = frameCounter;
                }
            }
            else if (timeStamp > 0 && active) {
                if (pDNA == dna->particles.size() - 1) {
                    addElems.push_back(Compound(*dna, protein->pos, {protein->linearVel.Y, -protein->linearVel.X}));
                    globalTimer.deleteEntry(timeStamp);

                    active = false;
                    globalTimer.recordNow();
                    timeStamp = frameCounter;
                } else {
                    f32_t currentX = protein->pos.X + protein->particles[pProtein].pos.X;
                    f32_t currentY = protein->pos.Y + protein->particles[pProtein].pos.Y;
                    Particle &p = dna->particles[pDNA];
                    f64_t theta = dna->angularVel * transcrMultiplier;
                    f64_t s = std::sin(theta);
                    f64_t c = std::cos(theta);
                    f32_t futureX = dna->pos.X + (dna->linearVel.X * transcrMultiplier) + (p.pos.X * c - p.pos.Y * s);
                    f32_t futureY = dna->pos.Y + (dna->linearVel.Y * transcrMultiplier) + (p.pos.X * s + p.pos.Y * c);
                    protein->linearVel = {(futureX - currentX) * transcrMultiplier, (futureY - currentY) * transcrMultiplier};
                }
            }
            else if (globalTimer.Read(timeStamp) > proteinInactivityTime) {
                globalTimer.deleteEntry(timeStamp);

                active = true;
                timeStamp = 0;
            }

            return NO_INTERACTION_TYPE;
        };

    //  Add your own interaction function here
    //  Format:
    //  
    //  case CompoundInteractionType::INTERACTION_ID:
    //      return [id, timeStamp, active](Compound *currentCompound, int particleId, Compound *touchedCompound, int touchedCompoundParticleId) mutable {
    //          if (!currentCompound) return id;
    //
    //          >Code comes here<
    //
    //          return NO_INTERACTION_TYPE;
    //      };





    // End of custom interaction function section

    default:
        CompoundInteractionType nullId = NO_INTERACTION_TYPE;
        return [nullId](Compound*, int, Compound*, int) mutable { return NO_INTERACTION_TYPE; };
    }
}
