#include "native_retarget.h"
#include <iostream>
#include <vector>
using namespace qte;
static void check(bool value,const char* text){if(!value)throw std::runtime_error(text);}
static uint32_t expected_witness;static NativeVerdict desired;static NativePlan expected_plans[2];static unsigned releases,aux_releases,prepares;static uint32_t native_answer,alignment_case;
static void __fastcall release_actor(void* witness,void*,void* actor){
    check(uint32_t(witness)==expected_witness,"Native release ECX/stack ABI");++releases;
    check(retarget::at<uint32_t>(uint32_t(actor),0x688)==expected_witness,"Foreign native registration released");retarget::at<uint32_t>(uint32_t(actor),0x688)=0;
}
static void __fastcall release_aux(void* witness,void*){check(uint32_t(witness)==expected_witness&&releases==0,"Aux release ordering/ABI");++aux_releases;retarget::at<uint32_t>(uint32_t(witness),0xfc)=0;}
static uint32_t __fastcall prepare(void* witness,void*){
    ++prepares;check(releases==2&&aux_releases==1,"Original preparation before native cleanup");
    check(retarget::at<uint8_t>(expected_witness,0xd)==1,"Native activation was not rearmed after initial preparation");
    for(auto plan:expected_plans){check(retarget::at<uint32_t>(plan.actor,0x1d0)==plan.state&&retarget::at<uint32_t>(plan.actor,0x1d4)==plan.clip,"Original planner context not restored for variant selection");retarget::at<uint32_t>(plan.actor,0x688)=alignment_case==2?expected_witness+4:alignment_case?0:expected_witness;retarget::at<uint32_t>(plan.actor,0x1d0)=20;retarget::at<uint32_t>(plan.actor,0x1d4)=250;}
    retarget::at<uint8_t>(expected_witness,0xd)=0;
    auto address=uint32_t(witness);retarget::at<uint32_t>(address,0x80)=desired.outcome;retarget::at<uint32_t>(address,0x84)=desired.attacker_address;retarget::at<uint32_t>(address,0x88)=desired.defender_address;retarget::at<uint8_t>(address,0xc)=alignment_case?0:1;
    if(alignment_case){
        retarget::at<uint8_t>(address,0xd0)=1;
        retarget::at<uint32_t>(address,0x98)=alignment_case==3?0:0x4000;
        if(alignment_case==4)retarget::at<uint32_t>(address,0x34)=0;
    }
    return native_answer;
}
int main(){try{
    std::vector<unsigned char> witness(0x110),one(0x700),two(0x700);uint32_t w=uint32_t(witness.data()),a=uint32_t(one.data()),b=uint32_t(two.data());expected_witness=w;
    expected_plans[0]={a,16,3};expected_plans[1]={b,18,4};retarget::Ports ports{reinterpret_cast<NativeWitnessCall>(prepare),reinterpret_cast<retarget::ReleaseActor>(release_actor),reinterpret_cast<retarget::ReleaseAux>(release_aux),0x10000000};
    auto reset=[&](uint32_t outcome){releases=aux_releases=prepares=0;native_answer=alignment_case=0;
        retarget::at<uint8_t>(w,0xc)=1;retarget::at<uint8_t>(w,0xd1)=0;retarget::at<uint8_t>(w,0xee)=0;retarget::at<uint32_t>(w,0x38)=2;retarget::at<uint32_t>(w,0x48)=0;
        retarget::at<uint32_t>(w,0x30)=a;retarget::at<uint32_t>(w,0x34)=b;retarget::at<uint32_t>(w,0x80)=outcome;retarget::at<uint32_t>(w,0xfc)=123;
        retarget::at<uint32_t>(w,0x24)=2;retarget::at<uint32_t>(w,0x84)=a;retarget::at<uint32_t>(w,0x88)=b;
        for(auto actor:{a,b}){retarget::at<uint32_t>(actor,0)=ports.engine+0x15dfec8;retarget::at<uint8_t>(actor,0xf0)=1;retarget::at<uint32_t>(actor,0x688)=w;retarget::at<uint32_t>(actor,0x1d0)=27;retarget::at<uint32_t>(actor,0x1d4)=8;retarget::at<uint32_t>(actor,0x1c8)=actor;}
    };
    for(uint32_t outcome:{0u,5u}){reset(outcome?0:5);desired={outcome,a,b};uint32_t stage=0;check(retarget::rebuild(w,desired,expected_plans,ports,stage)==0&&stage==6&&prepares==1,"Native variant transaction failed");
        check(retarget::at<uint32_t>(a,0x1d0)==20&&retarget::at<uint32_t>(b,0x1d4)==250,"Successful native planner output overwritten");}
    reset(5);desired={5,a,b};uint32_t stage=0;check(retarget::rebuild(w,desired,expected_plans,ports,stage)==0&&stage==6&&prepares==1&&releases==2&&aux_releases==1,"Unchanged variant failed to refresh native alignment preparation");
    for(unsigned variant=1;variant<=4;++variant){
        reset(5);desired={0,a,b};alignment_case=variant;
        const auto result=retarget::rebuild(w,desired,expected_plans,ports,stage);
        check(variant==1?result==0&&stage==8:result==1&&stage==5,
              "Deferred native alignment accepted a foreign link/variant/participant, or dropped the original pair");
    }
    for(unsigned bad=0;bad<5;++bad){reset(5);desired={0,a,b};
        if(bad==0)retarget::at<uint32_t>(b,0x688)=w+4;if(bad==1)retarget::at<uint8_t>(b,0xf0)=0;if(bad==2)retarget::at<uint32_t>(w,0x48)=1;if(bad==3)retarget::at<uint8_t>(w,0xd1)=1;if(bad==4)desired.defender_address=a;
        check(retarget::rebuild(w,desired,expected_plans,ports,stage)==1&&!prepares&&!releases&&!aux_releases,"Invalid/competing/playing encounter mutated");}
    reset(5);desired={0,a,b};native_answer=0x12340001;check(retarget::rebuild(w,desired,expected_plans,ports,stage)==native_answer&&retarget::at<uint32_t>(a,0x1d0)==27,"Native rejection or plan restoration lost");
    std::vector<unsigned char> rider_bytes(0x710),unit_bytes(0x1000);uint32_t rider=uint32_t(rider_bytes.data()),unit_record=uint32_t(unit_bytes.data()),passenger=rider;
    for(uint32_t outcome:{0u,5u}){
        reset(0);desired={outcome,a,b};expected_plans[0].kind=Horse;
        retarget::at<uint32_t>(a,0)=ports.engine+0x15e8524;retarget::at<uint32_t>(a,0x214)=unit_record;
        retarget::at<uint32_t>(unit_record,4)=42;retarget::at<uint32_t>(a,0x28)=123;
        retarget::at<uint32_t>(a,0x6cc)=1;retarget::at<uint32_t>(a,0x6d0)=uint32_t(&passenger);
        retarget::at<uint32_t>(rider,0)=ports.engine+0x15dfec8;retarget::at<uint8_t>(rider,0xf0)=1;
        retarget::at<uint32_t>(rider,0x214)=unit_record;retarget::at<uint32_t>(rider,0x210)=a;retarget::at<uint32_t>(rider,0x28)=123;
        check(retarget::rebuild(w,desired,expected_plans,ports,stage)==0&&stage==6&&releases==2&&prepares==1,"Native horse charge variant could not be rebuilt");
        check(retarget::at<uint32_t>(a,0x1d0)==20&&retarget::at<uint32_t>(a,0x1d4)==250,"Horse native planner output overwritten");
    }
    expected_plans[0].kind=Human;
    for(auto role152:{0x15dfec8u,0x15dbd68u,0x15dbfb4u}){
        reset(0);desired={5,b,a};retarget::at<uint32_t>(a,0)=ports.engine+role152;
        check(retarget::rebuild(w,desired,expected_plans,ports,stage)==1&&!prepares&&!releases,"Ordinary QTE accepted swapped roles");
        check(retarget::rebuild(w,desired,expected_plans,ports,stage,true)==0&&stage==6&&prepares==1&&releases==2&&retarget::at<uint32_t>(w,0x88)==a,"Routing-only original pair role reversal failed");
    }

    std::cout<<"PASS: native retarget production transaction, ECX/RET4 ABI, cleanup ordering, human and verified single-rider horse outcome variants, captured plans restored, playing/dead/extra/foreign participant guards and full native rejection return. Local synthetic objects only.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
