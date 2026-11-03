#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <atomic>
#include <chrono>
namespace qte {
enum class Action:uint32_t {Attack=1,Defend=2,Charge=3};
inline bool attacking(Action action){return action==Action::Attack||action==Action::Charge;}
enum class Button:uint32_t {Left=1,Right=2};
// Success = green zone. Early/Late are no longer produced by the zoned bar.
enum class Result:uint32_t {Pending=0,Success=1,Early=2,Late=3,WrongButton=4,Timeout=5,Canceled=6,Purple=7,Orange=8,Red=9,Blue=10};
// ---------------------------------------------------------------------------------------------
// ZONED BAR (user design). All values are fractions of the bar width, mirrored around the centre.
//   0.00-0.09  red end zone                 (18% total)
//   0.09-0.45  orange RNG zone (60% total) holding one moving blue zone and a 1% red strip
//              blue: 0.06 wide (12% total), moves on every new bar and every time the marker
//              reaches an end; red strip fixed at 0.295-0.305 (2% total)
//   0.45-0.47  purple (4% total)
//   0.47-0.53  green kill zone (6%)
// ---------------------------------------------------------------------------------------------
constexpr float RedEnd=.09f,BlueWidth=.06f,RedStripLow=.295f,RedStripHigh=.305f,PurpleHalf=.05f,GreenHalf=.03f;
constexpr float GreenWidth=2.f*GreenHalf;
// Marker: 25% faster than the old neutral sweep (1.475 s), 5% faster after every full crossing,
// 5% slower at the ends and 5% faster in the middle of each crossing.
constexpr float BaseSweepSeconds=1.475f/1.25f;
constexpr double SweepSpeedup=1.05,EaseAmplitude=.05;
// Outcome tuning (chances 0-1). "Buffs" are applied as changes to the odds of the game's own roll.
constexpr float GreenAttackKill=.85f;          // attack in green: 85% kill, never die (no game roll)
constexpr float OrangeDefendBuff=.25f;          // guard in orange: +25% defence, then the game rolls
constexpr float PurpleDefendBuff=.50f;          // guard in purple: +50% defence, then the game rolls
constexpr float PurpleAttackEnemyBuff=.25f;     // attack in purple: +25% enemy attack, then the game rolls
constexpr float RedDefendSurvive=.15f;          // guard in red: 85% die, 15% survive (no game roll)
constexpr float RedAttackDebuff=.85f;           // attack in red: the attack fails, and your NEXT exchange that uses the
                                                // game's roll is turned against you 85% of the time (stats -85%)
enum class Zone:uint32_t {Red,Blue,Orange,Purple,Green};
// Blue reverses the keys: the guard button counts as the attack and the attack button as the guard.
// Blue has no knockdown: after the reversed key it simply uses the game's roll.
inline uint64_t mix64(uint64_t v){v+=0x9e3779b97f4a7c15ull;v=(v^(v>>30))*0xbf58476d1ce4e5b9ull;v=(v^(v>>27))*0x94d049bb133111ebull;return v^(v>>31);}
// Start of a blue zone, measured from its own end of the bar. Never overlaps the red strip.
inline float blue_start(uint64_t serial,uint32_t sweep,bool right){
    constexpr float a0=RedEnd,a1=RedStripLow-BlueWidth,b0=RedStripHigh,b1=.5f-PurpleHalf-BlueWidth;
    const double u=double(mix64(serial*0x2545f4914f6cdd1dull^(uint64_t(sweep)<<1)^uint64_t(right))>>11)*(1./9007199254740992.);
    const float pick=float(u)*((a1-a0)+(b1-b0));
    return pick<(a1-a0)?a0+pick:b0+(pick-(a1-a0));
}
inline Zone zone_at(float x,uint64_t serial,uint32_t sweep){
    x=std::clamp(x,0.f,1.f);
    const float centre=std::abs(x-.5f);
    if(centre<=GreenHalf)return Zone::Green;
    if(centre<=PurpleHalf)return Zone::Purple;
    const bool right=x>.5f;const float from_end=right?1.f-x:x;
    if(from_end<RedEnd)return Zone::Red;
    const float b=blue_start(serial,sweep,right);
    if(from_end>=b&&from_end<b+BlueWidth)return Zone::Blue;
    if(from_end>=RedStripLow&&from_end<RedStripHigh)return Zone::Red;
    return Zone::Orange;
}
inline Result zone_result(Zone z){
    switch(z){case Zone::Green:return Result::Success;case Zone::Purple:return Result::Purple;
        case Zone::Orange:return Result::Orange;case Zone::Blue:return Result::Blue;default:return Result::Red;}
}
enum Fact:uint32_t {Fatigue=1,Morale=2,Elevation=4,General=8,Strength=16,Environment=32,EffectiveCombat=64};
constexpr uint32_t Required=Fatigue|Morale|Elevation|General;
struct UnitState {
    uint32_t known{},fatigue{},morale{};
    float height{},general_support{},strength=1.f;
    bool routing{};
    float environment_support{};
    float combat_score{};
};
struct Pair {
    uint64_t battle{},lease{};
    uint32_t player{},opponent{},player_unit{},opponent_unit{};
    // Native actor serial zero is valid (the possession lease uses it too).
    // Battle/lease identity and two distinct proven unit/actor serials guard
    // emptiness; a truthiness test would exclude the first native soldier.
    bool valid()const{return battle&&lease&&player!=opponent&&player_unit&&opponent_unit&&player_unit!=opponent_unit;}
};
inline bool same_pair(const Pair& a,const Pair& b){return a.battle==b.battle&&a.lease==b.lease&&a.player==b.player&&a.opponent==b.opponent&&a.player_unit==b.player_unit&&a.opponent_unit==b.opponent_unit;}
inline bool valid(const UnitState& s){
    return (s.known&Required)==Required&&s.fatigue<=5&&s.morale<=7&&std::isfinite(s.height)&&std::abs(s.height)<100000&&
        std::isfinite(s.general_support)&&s.general_support>=0&&s.general_support<=1&&
        (!(s.known&Strength)||(std::isfinite(s.strength)&&s.strength>=0&&s.strength<=1))&&
        (!(s.known&Environment)||(std::isfinite(s.environment_support)&&s.environment_support>=-1&&s.environment_support<=1))&&
        (!(s.known&EffectiveCombat)||(std::isfinite(s.combat_score)&&std::abs(s.combat_score)<=10000));
}
struct Difficulty {float advantage{},duration{},ideal{},tolerance{};};
inline bool difficulty(const UnitState& own,const UnitState& enemy,Difficulty& out){
    if(!valid(own)||!valid(enemy)||own.routing)return false;
    // Positive always means a relative advantage for the player's unit.
    // Units use their aggregate native fatigue/morale, not soldier counters.
    float advantage=.30f*float(int(enemy.fatigue)-int(own.fatigue))/5.f+
        .30f*float(int(enemy.morale)-int(own.morale))/7.f+
        .20f*std::clamp((own.height-enemy.height)/4.f,-1.f,1.f)+
        .20f*(own.general_support-enemy.general_support);
    if((own.known&enemy.known&Strength)!=0)advantage+=.10f*(own.strength-enemy.strength);
    if((own.known&enemy.known&Environment)!=0)advantage+=.10f*(own.environment_support-enemy.environment_support);
    if((own.known&enemy.known&EffectiveCombat)!=0){
        // Engine margin already includes physical combat modifiers. Avoid
        // counting its height/fatigue terms a second time at full strength.
        advantage=.70f*std::clamp((own.combat_score-enemy.combat_score)/40.f,-1.f,1.f)+
            .15f*float(int(enemy.morale)-int(own.morale))/7.f+
            .05f*(own.general_support-enemy.general_support)+
            .05f*(own.strength-enemy.strength)+.05f*(own.environment_support-enemy.environment_support);
    }
    if(enemy.routing)advantage+=.15f;
    advantage=std::clamp(advantage,-1.f,1.f);
    float t=(advantage+1.f)*.5f;
    out={advantage,.75f+1.45f*t,0,.055f+.145f*t};
    // Absolute distress remains hard even against an equally distressed unit.
    float distress=own.morale>=5?.30f:own.morale==4?.68f:1.f;
    if(own.morale>=6)distress=.22f;
    float exhaustion=1.f-.035f*float(own.fatigue);
    out.duration=std::max(.30f,out.duration*distress*exhaustion);
    out.tolerance=std::max(.012f,out.tolerance*distress*distress*exhaustion);
    out.ideal=out.duration*.80f;
    return true;
}
struct Snapshot {
    uint64_t serial{};
    Action action=Action::Attack;
    Result result=Result::Canceled;
    float elapsed{},duration{},ideal{},tolerance{},advantage{};
    uint32_t opponent{},opponent_unit{};
    bool active{},paused{},aligning{};
    float placement=.5f,zone_width{};
    // Separate event age from the phase clock, which rescales on speed changes.
    double response_elapsed{};
};
// Keep the established modifier function above as the source of weights and
// limits. The zero counterpart removes the other side's contribution; it is
// algebra only and is never sent to the native engine as an actor/context.
struct MeterDifficulty {Difficulty player{},enemy{},combined{};float width{};};
inline bool meter_difficulty(const UnitState& own,const UnitState& enemy,MeterDifficulty& out){
    if(!valid(own)||!valid(enemy)||own.routing)return false;
    UnitState zero_own{},zero_enemy{};
    zero_own.known=zero_enemy.known=Required|(own.known&enemy.known&(Strength|Environment|EffectiveCombat));
    zero_own.strength=zero_enemy.strength=0;
    // Relative elevation remains encounter context in the fallback branch.
    zero_own.height=own.height;zero_enemy.height=enemy.height;
    MeterDifficulty next;
    if(!difficulty(own,zero_enemy,next.player)||!difficulty(zero_own,enemy,next.enemy)||
       !difficulty(own,enemy,next.combined))return false;
    UnitState neutral{};neutral.known=Required;
    Difficulty baseline;if(!difficulty(neutral,neutral,baseline))return false;
    next.width=2.f*next.enemy.tolerance/baseline.duration;
    if(!std::isfinite(next.width)||next.width<=0||next.width>=1)return false;
    out=next;return true;
}
inline float random_placement()noexcept{
    // Separate entropy from difficulty. One draw per admitted encounter; no
    // random numbers enter speed, width, native selection or outcome replay.
    static std::atomic<uint64_t> sequence{[]()noexcept{
        try{return (uint64_t(std::random_device{}())<<32)^std::random_device{}();}
        catch(...){return uint64_t(std::chrono::steady_clock::now().time_since_epoch().count());}
    }()};
    uint64_t value=sequence.fetch_add(0x9e3779b97f4a7c15ull,std::memory_order_relaxed);
    value=(value^(value>>30))*0xbf58476d1ce4e5b9ull;
    value=(value^(value>>27))*0x94d049bb133111ebull;value^=value>>31;
    return float((double(value>>32)+.5)/4294967296.);
}
inline float zone_left(const Snapshot& s){return .5f-s.zone_width*.5f;} // green zone, centred
inline float zone_center(const Snapshot& s){return zone_left(s)+s.zone_width*.5f;}
// Which crossing the marker is on (0,1,2...) and how far through it (0-1). Crossing k lasts
// duration/1.05^k, so each full crossing is 5% faster than the one before.
inline void sweep_state(double elapsed,double duration,uint32_t& sweep,double& progress){
    sweep=0;progress=0;
    if(!std::isfinite(elapsed)||!std::isfinite(duration)||duration<=0||elapsed<=0)return;
    const double r=1./SweepSpeedup,x=1.-elapsed*(1.-r)/duration;
    double k=x>1e-12?std::floor(std::log(x)/std::log(r)):4000.;
    k=std::clamp(k,0.,4000.);
    const double start=duration*(1.-std::pow(r,k))/(1.-r),length=duration*std::pow(r,k);
    sweep=uint32_t(k);progress=std::clamp((elapsed-start)/length,0.,1.);
}
inline uint32_t sweep_index(const Snapshot& s){uint32_t k;double u;sweep_state(s.elapsed,s.duration,k,u);return k;}
inline float reticle_position(const Snapshot& s){
    uint32_t k;double u;sweep_state(s.elapsed,s.duration,k,u);
    // Speed is 0.95x at the ends and 1.05x in the middle of each crossing.
    const double pi=3.14159265358979323846,eased=u-EaseAmplitude*std::sin(2.*pi*u)/(2.*pi);
    return float((k&1u)?1.-eased:eased);
}
inline Zone current_zone(const Snapshot& s){return zone_at(reticle_position(s),s.serial,sweep_index(s));}
// At least three active-simulation seconds AND one complete outward/return
// cycle. Phase is preserved through stat refreshes, so speed changes cannot
// restart the required cycle or revive an expired event. No protocol field.
constexpr double ResponseWindowSeconds91=3.; // Retained minimum, not a hard cap.
constexpr double MaximumResponseSeconds95=4.42; // Existing duration <=2.21.
inline double response_remaining95(const Snapshot& s){
    if(s.result==Result::Timeout)return 0;
    return std::max({0.,ResponseWindowSeconds91-s.response_elapsed,
        2.*double(s.duration)-double(s.elapsed)});
}
inline double response_deadline(const Snapshot& s){return s.response_elapsed+response_remaining95(s);}
inline float response_remaining(const Snapshot& s){return float(response_remaining95(s));}
inline void project_response(Snapshot& s,double dt){
    if(s.result!=Result::Pending||s.paused||!std::isfinite(dt)||dt<0)return;
    const double remaining=response_remaining95(s);
    const double step=std::min(dt,remaining);
    s.elapsed=float(double(s.elapsed)+step);s.response_elapsed+=step;
    if(dt>=remaining)s.result=Result::Timeout;
}
inline bool in_success_zone(const Snapshot& s){
    float position=reticle_position(s),left=zone_left(s);
    return position>=left&&position<=left+s.zone_width;
}
class Event {
    uint64_t serial_{};
    Pair pair_{};
    Action action_=Action::Attack;
    Difficulty difficulty_{};
    double elapsed_{},response_elapsed_{};
    Result result_=Result::Canceled;
    bool active_{},paused_{};
    float placement_{},width_{};
    void geometry(){
        difficulty_.duration=BaseSweepSeconds;
        difficulty_.ideal=.5f*difficulty_.duration;
        difficulty_.tolerance=width_*.5f*difficulty_.duration;
    }
public:
    bool begin(const Pair& pair,Action action,const UnitState& own,const UnitState& enemy,float placement=random_placement()){
        MeterDifficulty d;
        if(active_||serial_==UINT64_MAX||!pair.valid()||!meter_difficulty(own,enemy,d)||
           !std::isfinite(placement)||placement<0||placement>1)return false;
        if(!attacking(action)&&action!=Action::Defend)return false;
        ++serial_;pair_=pair;action_=action;difficulty_=d.player;difficulty_.advantage=d.combined.advantage;
        placement_=.5f;width_=GreenWidth;(void)placement;geometry();elapsed_=0;response_elapsed_=0;result_=Result::Pending;active_=true;paused_=false;return true;
    }
    bool update(const Pair& pair,const UnitState& own,const UnitState& enemy,double simulation_dt,bool paused){
        if(!active_)return false;
        if(!same_pair(pair,pair_)||!pair.valid()||!valid(own)||!valid(enemy)||own.routing||!std::isfinite(simulation_dt)||simulation_dt<0){cancel();return false;}
        paused_=paused;
        if(result_!=Result::Pending||paused)return true;
        // Preserve the current forward/return phase through speed changes.
        // The placement quantile is stable while native enemy width refreshes.
        MeterDifficulty changed;if(!meter_difficulty(own,enemy,changed)){cancel();return false;}
        double phase=elapsed_/difficulty_.duration;
        difficulty_.advantage=changed.combined.advantage; // fixed bar: speed and zones ignore unit stats
        width_=GreenWidth;geometry();
        // Preserve phase on stat refresh without changing consumed response time.
        const double remaining=std::max({0.,ResponseWindowSeconds91-response_elapsed_,(2.-phase)*double(difficulty_.duration)});
        const double step=std::min(simulation_dt,remaining);
        elapsed_=phase*difficulty_.duration+step;response_elapsed_+=step;
        if(simulation_dt>=remaining)result_=Result::Timeout;
        return true;
    }
    Result press(uint64_t serial,Button button,bool down_edge){
        if(!active_||paused_||serial!=serial_||!down_edge||result_!=Result::Pending)return result_;
        const Zone zone=current_zone(snapshot());
        Button expected=attacking(action_)?Button::Left:Button::Right;
        if(zone==Zone::Blue)expected=expected==Button::Left?Button::Right:Button::Left;
        result_=button!=expected?Result::WrongButton:zone_result(zone);
        return result_;
    }
    Result press_visible93(uint64_t serial,Button button,bool edge,const Snapshot& shown){
        if(!active_||paused_||result_!=Result::Pending||!edge||serial!=serial_)return result_;
        // Only a fresh frame actually submitted by our renderer may supply phase.
        // The authoritative response clock and lifecycle still decide eligibility.
        if(shown.serial!=serial_||shown.action!=action_||shown.opponent!=pair_.opponent||shown.opponent_unit!=pair_.opponent_unit||
           !shown.active||shown.paused||shown.result!=Result::Pending||!std::isfinite(shown.elapsed)||shown.elapsed<0||
           !std::isfinite(shown.duration)||shown.duration<.30f||shown.duration>2.21f||
           !std::isfinite(shown.placement)||shown.placement<0||shown.placement>1||
           !std::isfinite(shown.zone_width)||shown.zone_width<=0||shown.zone_width>=1||
           !std::isfinite(shown.response_elapsed)||shown.response_elapsed<0||shown.response_elapsed>=response_deadline(shown)||
           shown.response_elapsed>response_elapsed_+1e-4||response_elapsed_-shown.response_elapsed>1.6)return result_;
        // No delay: judge the live marker at the moment of the click, not the last drawn frame.
        const Zone zone=current_zone(snapshot());
        Button expected=attacking(action_)?Button::Left:Button::Right;
        if(zone==Zone::Blue)expected=expected==Button::Left?Button::Right:Button::Left; // reversed keys
        result_=button!=expected?Result::WrongButton:zone_result(zone);
        return result_;
    }
    void cancel(){active_=false;paused_=false;result_=Result::Canceled;}
    bool consume(const Pair& pair,Result& result){
        if(!active_||result_==Result::Pending||!same_pair(pair,pair_))return false;
        result=result_;active_=false;paused_=false;return true;
    }
    Snapshot snapshot()const{return {serial_,action_,result_,float(elapsed_),difficulty_.duration,difficulty_.ideal,difficulty_.tolerance,difficulty_.advantage,pair_.opponent,pair_.opponent_unit,active_,paused_,false,placement_,width_,response_elapsed_};}
};
enum class CombatIntent {Stock,Blocked,Hit};
inline float outcome_roll()noexcept{return random_placement();}
// Hit = the swing in this pair lands (the player's when attacking, the enemy's when guarding).
// stock_hit = the game's own pre-rolled verdict was a landed swing.
// Set by an attack in red; used up by the player's next resolved exchange.
inline std::atomic<uint32_t> red_attack_debuff{0};
inline CombatIntent zone_intent(Action action,Result result,bool stock_hit);
inline CombatIntent combat_intent(Action action,Result result,bool stock_hit=false){
    if(result==Result::Canceled||result==Result::Pending)return CombatIntent::Stock; // interrupted: game's own result, debuff kept
    const bool debuffed=red_attack_debuff.exchange(0)!=0;
    const CombatIntent intent=zone_intent(action,result,stock_hit);
    if(attacking(action)&&result==Result::Red)red_attack_debuff.store(1);
    // The debuff only touches results that come from the game's roll (orange, purple, blue, no click).
    const bool game_roll=result==Result::Orange||result==Result::Purple||result==Result::Blue||result==Result::Timeout;
    if(debuffed&&game_roll&&outcome_roll()<RedAttackDebuff)
        return attacking(action)?CombatIntent::Blocked:CombatIntent::Hit;
    return intent;
}
inline CombatIntent zone_intent(Action action,Result result,bool stock_hit){
    if(result==Result::Timeout)return CombatIntent::Stock; // no click: game's roll
    const auto stock=[&]{return CombatIntent::Stock;};
    if(attacking(action)){
        switch(result){
        case Result::Success: return outcome_roll()<GreenAttackKill?CombatIntent::Hit:CombatIntent::Blocked; // never die
        case Result::Orange:  return stock();
        case Result::Purple:  return stock_hit&&outcome_roll()<PurpleAttackEnemyBuff?CombatIntent::Blocked:stock();
        case Result::Blue:    return stock(); // reversed key accepted: game's roll
        // Red: an attack exchange has no result where the player dies, so the attack fails and the
        // red-attack debuff above makes the player's next game-roll exchange go against them 85% of the time.
        default:              return CombatIntent::Blocked; // red / wrong button
        }
    }
    switch(result){
    case Result::Success: return CombatIntent::Blocked; // guaranteed guard
    case Result::Orange:  return stock_hit&&outcome_roll()<OrangeDefendBuff?CombatIntent::Blocked:stock();
    case Result::Purple:  return stock_hit&&outcome_roll()<PurpleDefendBuff?CombatIntent::Blocked:stock();
    case Result::Red:     return outcome_roll()<RedDefendSurvive?CombatIntent::Blocked:CombatIntent::Hit;
    case Result::Blue:    return stock(); // reversed key accepted: game's roll
    default:              return CombatIntent::Hit; // wrong button
    }
}
// Production must supply VERIFIED native animation/result bindings. Fixture
// codes do not certify real block/hit behavior. In particular, code5 reaches
// a native fatal path; never guess that it is a harmless hit or use it by default.
struct OutcomeBindings {
    uint32_t blocked=UINT32_MAX,hit=UINT32_MAX;bool verified{};
    bool valid()const{return verified&&blocked<=5&&hit<=5&&blocked!=hit;}
};
inline bool native_outcome(Action action,Result result,uint32_t original,
                           const OutcomeBindings& bindings,uint32_t& output){
    auto intent=combat_intent(action,result,bindings.valid()&&original==bindings.hit);
    if(intent==CombatIntent::Stock){output=original;return true;}
    if(!bindings.valid())return false;
    output=intent==CombatIntent::Hit?bindings.hit:bindings.blocked;return true;
}
inline float closing_radius(const Snapshot& s,float target=32.f,float outer=116.f){
    if(s.ideal<=0||!std::isfinite(s.elapsed))return outer;
    return std::max(0.f,outer+(target-outer)*s.elapsed/s.ideal);
}
}
