package main;

use strict;
use warnings;
use POSIX;
#use Switch;

sub
Asksin_SmartReader_Initialize($$)
{
  my ($hash) = @_;
}

sub
registerSmartReader()
{
  {$HMConfig::culHmModel{"8FFE"} = {name=>"SmartReader",st=>'smartReader',cyc=>'',rxt=>'',lst=>'1',chn=>""}};
  {$HMConfig::culHmRegType{"smartReader"} = {readoutInterval=>1}};
  {$HMConfig::culHmChanSets{"SmartReader00"} = {"m3"=>"","kWh"=>""}};
  {$HMConfig::culHmRegDefine{"readoutInterval"} = {a=>13.0,s=>4.0,l=>0,min=>0,max=>4294967296,c=>'',f=>'',u=>'ms',d=>1,t=>'Interval for meter readout'}};
  #{$HMConfig::culHmRegChan{"SmartReader00"}  = $HMConfig::culHmRegType{remote}};

  #create literal inverse for fast search
  ###my $rN = "readoutInterval";
  ###foreach my $lit (keys %{$HMConfig::culHmRegDefine{$rN}{lit}}){
  ###  #Log(1, "Registered $lit");
  ###  $HMConfig::culHmRegDefine{$rN}{litInv}{$HMConfig::culHmRegDefine{$rN}{lit}{$lit}}=$lit;
  ###}
  #Log(1, "inv is $HMConfig::culHmRegDefine{$rN}{litInv}{1}");


  #{$HMConfig::culHmChanSets{"HM-LC-Sw1PBU-FM-CustomFW01"} = $HMConfig::culHmSubTypeSets{"THSensor"}};
  #{$HMConfig::culHmChanSets{"HM-LC-Sw1PBU-FM-CustomFW02"} = $HMConfig::culHmSubTypeSets{"THSensor"}};
  #{$HMConfig::culHmChanSets{"HM-LC-Sw1PBU-FM-CustomFW03"} = $HMConfig::culHmSubTypeSets{"switch"}};
  #{$HMConfig::culHmChanSets{"HM-LC-Sw1PBU-FM-CustomFW04"} = $HMConfig::culHmSubTypeSets{"switch"}};
  #{$HMConfig::culHmRegChan{"HM-LC-Sw1PBU-FM-CustomFW01"}  = $HMConfig::culHmRegType{remote}};
  #{$HMConfig::culHmRegChan{"HM-LC-Sw1PBU-FM-CustomFW02"}  = $HMConfig::culHmRegType{remote}};
  #{$HMConfig::culHmRegChan{"HM-LC-Sw1PBU-FM-CustomFW03"}  = $HMConfig::culHmRegType{switch}};
  #{$HMConfig::culHmRegChan{"HM-LC-Sw1PBU-FM-CustomFW04"}  = $HMConfig::culHmRegType{switch}};
  #Log(1, "Registered F0A9");

  setReadingsVal(CUL_HM_name2Hash("HM_9A459F"), "retry-timer-running", "no", TimeNow());
}

#Log(1, "Loaded CustomFS");
InternalTimer(gettimeofday()+10,"registerSmartReader","nothing", 0);

sub CUL_HM_ParsesmartReader($$$$$$) {
  my($mFlg,$mTp,$src,$dst,$p,$target) = @_;
  my @evtEt = ();

  #Log 1,"General  entering with $mFlg,$mTp,$src,$dst,$p";

  if ($mTp eq "41" && $p =~ m/^0200/) { # handle readout
    #my $chn = "00";
    my $chId = $src;#.$chn;
    my $shash = $modules{CUL_HM}{defptr}{$chId} if($modules{CUL_HM}{defptr}{$chId});
    #print "$_\n" for keys %{$modules{CUL_HM}{defptr}};
    #Log 1,"chId is $chId, module is $shash";
    my ($kWh1,$kWh2,$kWh3,$kWh4,$m31,$m32,$m33,$m34) = (hex($1),hex($2),hex($3),hex($4),hex($5),hex($6),hex($7),hex($8)) if ($p =~ m/^....(..)(..)(..)(..)(..)(..)(..)(..)/);
    my $kWh = (($kWh4 * 256 + $kWh3) * 256 + $kWh2) * 256 + $kWh1;
    my $m3 = (($m34 * 256 + $m33) * 256 + $m32) * 256 + $m31;
    push @evtEt,[$shash,1,"kWh:$kWh"];
    push @evtEt,[$shash,1,"m3:$m3"];
    setReadingsVal($shash, "readout-pending", "no", TimeNow());
  } elsif (($mTp eq "02" && $p =~ m/^0100/)|| # ackStatus
           ($mTp eq "10" && $p =~ m/^0600/)) { # infoStatus
    my $chId = $src;#.$chn;
    my $shash = $modules{CUL_HM}{defptr}{$chId} if($modules{CUL_HM}{defptr}{$chId});
    my ($output) = ($1) if ($p =~ m/^....(..)/);
    my $showcase = "off";
    $showcase = "on" if ($output eq "00");
    push @evtEt,[$shash,1,"showcase:$showcase"];
    setShowcaseLightOn() if ($showcase eq "off" && ReadingsVal("HM_9A459F", "showcase-wanted", undef) eq "yes");
    setShowcaseLightOff() if ($showcase eq "on" && ReadingsVal("HM_9A459F", "showcase-wanted", undef) eq "no");
  } else {
    # ATTENTION: this goes to fhem-2017-03.log! not HM_9A459F.log!
    Log 1,"SmartReader unknown: $mFlg,$mTp,$src,$dst,$p";
  }

=pod
  if (($mTp eq "02" && $p =~ m/^01/) ||  # handle Ack_Status
      ($mTp eq "10" && $p =~ m/^06/)) { #    or Info_Status message here


    my $rSUpdt = 0;# require status update
    my ($subType,$chn,$val,$err) = ($1,hex($2),hex($3)/2,hex($4))
                        if($p =~ m/^(..)(..)(..)(..)/);
    $chn = sprintf("%02X",$chn&0x3f);
    my $chId = $src.$chn;
    my $shash = $modules{CUL_HM}{defptr}{$chId}
                           if($modules{CUL_HM}{defptr}{$chId});

    my $vs = ($val==100 ? "on":($val==0 ? "off":"$val %")); # user string...

    push @evtEt,[$shash,1,"level:$val %"];
    push @evtEt,[$shash,1,"pct:$val"];# duplicate to level - necessary for "slider"
    push @evtEt,[$shash,1,"deviceMsg:$vs$target"] if($chn ne "00");
    push @evtEt,[$shash,1,"state:".$vs];
    my $action; #determine action
    push @evtEt,[$shash,1,"timedOn:".(($err&0x40)?"running":"off")];
  }
  elsif ($mTp eq "5E" ||$mTp eq "5F" ) {  #    POWER_EVENT_CYCLIC

    my $shash = $modules{CUL_HM}{defptr}{$src."04"}
                             if($modules{CUL_HM}{defptr}{$src."04"});
    my ($eCnt,$P,$I,$U,$F) = unpack 'A6A6A4A4A2',$p;
#    push @evtEt,[$shash,1,"energy:"   .(hex($eCnt)&0x7fffff)/10];# 0.0  ..167772.15 W
#    push @evtEt,[$shash,1,"power:"    . hex($P   )/100];
    push @evtEt,[$shash,1,"current:"  . hex($I   )/1]# 0.0  ..65535.0   mA;
#    push @evtEt,[$shash,1,"voltage:"  . hex($U   )/10] # 0.0  ..6553.5    mV;
#    push @evtEt,[$shash,1,"frequency:".(hex($F   )/100+50)] # 48.72..51.27     Hz;
#    push @evtEt,[$shash,1,"boot:"     .((hex($eCnt)&0x800000)?"on":"off");];
  }
  elsif($mTp =~ m/^4./ && $p =~ m/^(..)(..)/) {
    my $shash = CUL_HM_id2Hash($src);
    my ($chn, $bno) = (hex($1), hex($2));# button number/event count
    my $buttonID = $chn&0x3f;# only 6 bit are valid
    my $btnName;
    my $state = "";
    my $chnHash = $modules{CUL_HM}{defptr}{$src.sprintf("%02X",$buttonID)};

    if ($chnHash){# use userdefined name - ignore this irritating on-off naming
      $btnName = $chnHash->{NAME};
    }
    else{# Button not defined, use default naming
      $chnHash = $shash;
      my $btn = int((($chn&0x3f)+1)/2);
      $btnName = "Btn$btn";
      $state = ($chn&1 ? "off" : "on")
    }
    my $trigType;
    if($chn & 0x40){
      if(!$shash->{BNO} || $shash->{BNO} ne $bno){#bno = event counter
        $shash->{BNO}=$bno;
        $shash->{BNOCNT}=0; # message counter reest
      }
      $shash->{BNOCNT}+=1;
      $state .= "Long" .(hex($mFlg) & 0x20 ? "Release" : "").
                " ".$shash->{BNOCNT}."-".$mFlg.$mTp."-";
      $trigType = "Long";
    }
    else{
      $state .= "Short";
      $trigType = "Short";
    }
    $shash->{helper}{addVal} = $chn;   #store to handle changesFread
    push @evtEt,[$chnHash,1,"state:".$state.$target];
    push @evtEt,[$chnHash,1,"trigger:".$trigType."_".$bno];
    push @evtEt,[$shash,1,"battery:". (($chn&0x80)?"low":"ok")];
    push @evtEt,[$shash,1,"state:$btnName $state$target"];
  } else {
    Log(1, "Asksin_SmartReader received unknown message: $mFlg,$mTp,$src,$dst,$p");
  }
=cut

  return @evtEt;
}

sub setShowcaseLightOff() {
  my $hash = CUL_HM_name2Hash("HM_9A459F");
  my $flag = CUL_HM_getFlag($hash);
  my $id = CUL_HM_IoId($hash);
  my $dst = substr($hash->{DEF},0,6);

  setReadingsVal($hash, "showcase-wanted", "no", TimeNow());
  CUL_HM_PushCmdStack($hash,"++$flag"."11$id$dst"."020001");
  CUL_HM_ProcessCmdStack($hash);
}

sub setShowcaseLightOn() {
  my $hash = CUL_HM_name2Hash("HM_9A459F");
  my $flag = CUL_HM_getFlag($hash);
  my $id = CUL_HM_IoId($hash);
  my $dst = substr($hash->{DEF},0,6);

  setReadingsVal($hash, "showcase-wanted", "yes", TimeNow());
  CUL_HM_PushCmdStack($hash,"++$flag"."11$id$dst"."020000");
  CUL_HM_ProcessCmdStack($hash);
}

sub requestMeterReadout() {
  my $hash = CUL_HM_name2Hash("HM_9A459F");
  my $flag = CUL_HM_getFlag($hash);
  my $id = CUL_HM_IoId($hash);
  my $dst = substr($hash->{DEF},0,6);

  setReadingsVal($hash, "readout-pending", "yes", TimeNow()) if (ReadingsVal("HM_9A459F", "readout-pending", "no") eq "no");
  CUL_HM_PushCmdStack($hash,"++$flag"."11$id$dst"."030000");
  CUL_HM_ProcessCmdStack($hash);

  # the device acks this command immediately and only sends an answer if the readout succeeded
  # we use this timer to restart the readout in case it failed
  startCheckShowcaseAndReadoutTimer();
}

sub startCheckShowcaseAndReadoutTimer() {
  return if (ReadingsVal("HM_9A459F", "retry-timer-running", "no") eq "yes");

  setReadingsVal(CUL_HM_name2Hash("HM_9A459F"), "retry-timer-running", "yes", TimeNow());
  InternalTimer(gettimeofday()+180, "checkShowcaseAndReadout", "nothing", 0);
}

sub checkShowcaseAndReadout() {
  setReadingsVal(CUL_HM_name2Hash("HM_9A459F"), "retry-timer-running", "no", TimeNow());
  my $wanted = ReadingsVal("HM_9A459F", "showcase-wanted", undef);
  my $cur = ReadingsVal("HM_9A459F", "showcase", undef);
  setShowcaseLightOn() if ($cur eq "off" && $wanted eq "yes");
  setShowcaseLightOff() if ($cur eq "on" && $wanted eq "no");

  if (ReadingsVal("HM_9A459F", "readout-pending", "no") eq "yes") {
    my ($seconds, $microseconds) = gettimeofday();
    requestMeterReadout() if ($seconds - time_str2num(ReadingsTimestamp("HM_9A459F", "readout-pending", "1970-01-01 01:00:00")) - 60 * 30 < 0);
  }
}

1;
