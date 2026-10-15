/* SPDX-License-Identifier: GPL-3.0-only */
#define FELUCCA_SAMPLER_ONLY 1
#define FELUCCA_KO_DUAL 1
#define KO_TEST_MAIN boot_base_main
#include "ko_test.c"
int main(void)
{
    int bad=0;ui_power_on();memset(ko_states,0,sizeof ko_states);
    memset(ko_flash,255,sizeof ko_flash);slc_store_boot();fixture(0,0);fixture(1,1);
    song.sel=2;ko_context=~0u;ko_states[1].on=1;
    trk[2].eng_req=13;trk[2].p[P_E0]=4;trk[2].p[P_E1]=0;
    trk[2].step[3].note[0]=67;trk[2].p[P_SLEN]=24;
    ko_product_boot();
    bad+=check("boot overrides restored Song 2 with Song 1",song.sel==0);
    bad+=check("both song contexts share enabled sampler state",ko_states[0].on&&ko_states[1].on);
    bad+=check("old source settings become the three product slots",ko_song(trk)&&ko_kit(trk+1)&&ko_song(trk+2));
    bad+=check("repair preserves sequence notes and length",trk[2].step[3].note[0]==67&&trk[2].p[P_SLEN]==24);
    song.sel=2;ui_input();
    bad+=check("switching to Song 2 cannot disable the shared sampler",ko_states[0].on&&ko_states[1].on&&ko_song(trk+2));
    trk_note_on(trk+2,60,100);int32_t audio[CTL*2];int silent=1;
    for(uint32_t n=0;n<100;n++){mix_block(audio,CTL);for(uint32_t i=0;i<CTL*2;i++)silent&=audio[i]==0;}
    bad+=check("empty Song 2 stays silent on the hardware path",silent);
    project_t project;project_capture(&project);project.sel=2;
    project.t[2].p[P_E0]=4;project.t[2].p[P_E1]=0;
    project.sum=proj_sum(&project);
    bad+=check("restoring a project repairs its legacy source",project_restore_runtime(&project)==0&&ko_song(trk+2));
    printf("PIGEON boot: %s\n",bad?"FAILED":"PASSED");return !!bad;
}
