// Exercise the actual camera consumer and actual aim-frame capture at every
// scroll notch, with native-world-query providers from verify_view152.
{
 unsigned cases203=0;third_person152=1;
 for(float heading:{-3.1f,-1.f,0.f,1.f,3.1f})for(float pitch:{-.9f,0.f,.9f})for(unsigned notch=0;notch<=22;++notch)for(bool blocked:{false,true}){
  view_distance152=1.f+notch*.5f;obstruct152=blocked;
  float forward[]={std::sin(heading)*std::cos(pitch),std::sin(pitch),std::cos(heading)*std::cos(pitch)};
  float right[]={std::cos(heading),0.f,-std::sin(heading)};
  float up[]={-std::sin(heading)*std::sin(pitch),std::cos(pitch),-std::cos(heading)*std::sin(pitch)};
  float pivot[]={4.f,5.f,6.f};for(unsigned i=0;i<3;++i){field<float>(cam,8+i*4)=pivot[i];field<float>(cam,0x14+i*4)=pivot[i]+forward[i];field<float>(cam,0x58+i*4)=up[i];}
  view_camera152(cam);capture_aim_frame(cam);ViewRay actual{};require(read_aim_frame(actual),"Scroll lost actual aim capture");
  float side=0,depth=0,height=0;
  for(unsigned i=0;i<3;++i){float relative=pivot[i]-actual.origin[i];side+=relative*right[i];depth+=relative*forward[i];height+=relative*up[i];require(std::abs(actual.direction[i]-forward[i])<.00001f,"Framing rotated aim");}
  require(depth>0&&std::abs(side/depth+.25f)<.0001f&&std::abs(height/depth+.08f)<.0001f,"Zoom/collision centred marker on soldier");
  ++cases203;
 }
 float forward[]={0,0,1},up[]={0,1,0},delta[3],length{};
 require(!framing203::segment(forward,up,0,delta,length)&&!framing203::segment(forward,up,13,delta,length),"Invalid camera distance accepted");
 require(framing203::fraction(-1,3)==0&&framing203::fraction(5,3)==1,"Collision segment escaped bounds");
 std::cout<<"PASS framing203: "<<cases203<<" actual camera/aim frames across all scroll notches, yaw, pitch and obstruction.\n";
}
