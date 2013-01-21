script SAMSARA_CLIENT_CLASS (int slot) clientside
{
    int toClass = SamsaraClientClass-1;
    int displaymode = GetCVar("samsara_cl_pickupmode");
    int oldslot = slot;
    slot = itemToSlot(slot);
    int hasSlot = SamsaraClientWeps[slot];
    
    if (displaymode != 0)
    {
        if ((SamsaraItemFlash >= (Timer() - 35)) && (Timer() >= 35))
        {
            Spawn("SamsaraChangeFlash", GetActorX(0), GetActorY(0), GetActorZ(0));
        }
        
        if (toClass == -1)
        {
            SetActorState(0, "NoGuy");
            terminate;
        }
    }
    
    if (slot == -1)
    {
        switch (displaymode)
        {
          case 0:
            SetActorState(0, "NoGuy");
            break;
            
          case 1:
            SetActorState(0, PickupStates[toClass][3]);
            break;
            
          case 2:
            SetActorState(0, PickupStates[toClass][0]);
            break;
        }
        
        terminate;
    }
    
    switch (displaymode)
    {
      case 0:
        SetActorState(0, "NoGuy");
        break;
        
      case 1:
        if ((SamsaraClientWepFlashes[slot] >= (Timer() - 35)) && (Timer() >= 35))
        {
            Spawn("SamsaraChangeFlash2", GetActorX(0), GetActorY(0), GetActorZ(0));
        }
        
        if (hasSlot) { SetActorState(0, PickupStates[toClass][1]); }
        else         { SetActorState(0, PickupStates[toClass][2]); }
        break;
        
      case 2:
        SetActorState(0, PickupStates[toClass][0]);
        break;
    }
}

/*
*      WARNING
*  This script is over 20 variables. Check here first for segfaults.
*  Also, keep an eye on this script for potential desync issues.
*  (current count: 25)
*/

script SAMSARA_GIVEWEAPON (int slot, int dropped, int silent)
{
    if (!IsServer) { terminate; }
    slot = itemToSlot(slot);
    
    
    int weaponStay = !!GetCVar("sv_weaponstay");
    int weaponGet  = 0;
    int pclass = samsaraClassNum();
    int hasWep = HasClassWeapon(pclass, slot);
    
    if (slot == SLOTCOUNT-1) { weaponStay &= !!GetCVar("samsara_permault"); }
    
    int a1cnt  = 0, a2cnt = 0;
    int a1max  = 0, a2max = 0;
    int a1max2 = 0, a2max2 = 0;
    int a1Full = 0, a2Full = 0;
    int a1diff = 0, a2diff = 0;
    
    int weapon  = ClassWeapons[pclass][slot][S_WEP],        wepbool = !!StrLen(weapon); 
    int ammo1   = ClassWeapons[pclass][slot][S_AMMO1],      a1bool  = !!StrLen(ammo1);
    int ammo2   = ClassWeapons[pclass][slot][S_AMMO2],      a2bool  = !!StrLen(ammo2);
    int check   = ClassWeapons[pclass][slot][S_CHECKITEM],  chkbool = !!StrLen(check);
    
    if (!wepbool || (CheckInventory(ClassWeapons[pclass][slot][S_CHECKFAILITEM]) && !dropped))
    {
        SetResultValue(weaponStay * WEPFLAGS_WEAPONSTAY);
        terminate;
    }
    
    if (a1bool)
    {
        a1cnt   = CheckInventory(ammo1);
        a1max   = GetAmmoCapacity(ammo1);
        a1max2  = a1max * 4; // ya never know
    }
    
    if (a2bool)
    {
        a2cnt   = CheckInventory(ammo2);
        a2max   = GetAmmoCapacity(ammo2);
        a2max2  = a1max * 4;
    }
    
    if (a1Bool) { a1Full = (CheckInventory(ammo1) == a1max); }
    if (a2Bool) { a2Full = (CheckInventory(ammo2) == a2max); }
    
    if (dropped && IsServer)
    {
        if (a1bool) { SetAmmoCapacity(ammo1, a1max2); }
        if (a2bool) { SetAmmoCapacity(ammo2, a2max2); }
    }
    
    // do we even have this?
    if (!hasWep || (chkbool && !CheckInventory(check)))
    {
        weaponGet = 1;
    }
    else if (!weaponStay || dropped)     // does this not stay on the ground?
    {
        if ((a1bool && !a1Full) || (a2Bool && !a2Full)) { weaponGet = 1; }
    }

    if (weaponGet && IsServer)
    {
        int success = !_giveclassweapon(pclass, slot, 3, dropped);

        if (!silent && success)
        {
            if (!hasWep)
            {
                Spawn("WeaponGetYaaaay", GetActorX(0), GetActorY(0), GetActorZ(0));
                Spawn("WeaponGetYaaaay2", GetActorX(0), GetActorY(0), GetActorZ(0));
            }

            ACS_ExecuteAlways(SAMSARA_CLIENT_WEAPONPICKUP, 0, slot,GetCVar("compat_silentpickup"),dropped);
        }
    }
    
    if (dropped && IsServer)
    {
        TakeInventory(ammo1, (CheckInventory(ammo1) - a1cnt) / 2);
        TakeInventory(ammo2, (CheckInventory(ammo2) - a2cnt) / 2);
        if (a1bool) { SetAmmoCapacity(ammo1, a1max); }
        if (a2bool) { SetAmmoCapacity(ammo2, a2max); }
    }
    
    if (IsServer)
    {
        TakeInventory(ammo1, CheckInventory(ammo1) - a1max);
        TakeInventory(ammo2, CheckInventory(ammo2) - a2max);
    }
    
    SetResultValue((weaponStay * WEPFLAGS_WEAPONSTAY) + (success * WEPFLAGS_GOTWEAPON));
}

script SAMSARA_GIVEUNIQUE (int alt)
{
    if (!IsServer) { terminate; }
    
    int uniqueGet = 0;
    int pclass = samsaraClassNum();
    
    while (!uniqueGet && alt >= 0)
    {
        uniqueGet = GiveUnique(pclass, alt);
        alt--;
    }
    
    if (uniqueGet && IsServer)
    {
        ACS_ExecuteAlways(SAMSARA_CLIENT_UNIQUEPICKUP, 0, GetCVar("compat_silentpickup"), 0, 0);
    }
    
    SetResultValue(uniqueGet);
}

int QuoteStorage[MSGCOUNT];

script SAMSARA_CLIENT_WEAPONPICKUP (int slot, int soundmode, int dropped) clientside
{
    int pln = PlayerNumber(), cpln = ConsolePlayerNumber();
    int pclass = samsaraClassNum();
    int i, j, quoteCount = 0;
    int logMsg;
    int pickupsound = ClassPickupSounds[pclass][slot];
    
    if (dropped) { pickupsound = ClassDropSounds[pclass][slot]; }
    
    if (cpln == pln)
    {
        if (GetCVar("samsara_cl_moremessages"))
        {
            for (i = 0; i < MSGCOUNT; i++)
            {
                j = ClassPickupMessages[pclass][slot][i];
                if (!StrLen(j)) { continue; }
                
                QuoteStorage[quoteCount++] = j;
            }
            
            if (!quoteCount) { logMsg = "Oh bugger there's no messages for this weapon."; }
            else { logMsg = QuoteStorage[random(0, quoteCount-1)]; }
        }
        else
        {
            logMsg = ClassPickupMessages[pclass][slot][0];
            
            if (!StrLen(logMsg)) { logMsg = "Oh bugger there's no message for this weapon."; } 
        }

        if (GetCVar("samsara_cl_printpickup")) { Print(s:logMsg); }
        else { Log(s:logMsg); }
    }
    
    if (soundmode == 1) { LocalAmbientSound(pickupsound, 127); }
    else { ActivatorSound(pickupsound, 127); }
    
    FadeRange(ClassFades[pclass][0], ClassFades[pclass][1], ClassFades[pclass][2], ClassFades[pclass][3],
    ClassFades[pclass][0], ClassFades[pclass][1], ClassFades[pclass][2], 0.0, itof(ClassFades[pclass][4]) / 35);
    
    if (pclass == CLASS_DUKE && !GetCVar("samsara_cl_ballgag") && !dropped)
    {
        Delay(8);

        if (!DukeQuoteCooldown[pln])
        {
            if (soundmode == 1) { LocalAmbientSound("duke/weapontaunt", 127); }
            else { ActivatorSound("duke/weapontaunt", 127); }
            DukeQuoteCooldown[pln] = 140;
        }
    }
}

script SAMSARA_CLIENT_UNIQUEPICKUP (int soundmode) clientside
{
    int pln = PlayerNumber(), cpln = ConsolePlayerNumber();
    int pclass = samsaraClassNum();
    int i, j, quoteCount = 0;
    int logMsg;
    
    if (cpln == pln)
    {
        if (GetCVar("samsara_cl_moremessages"))
        {
            for (i = 0; i < MSGCOUNT; i++)
            {
                j = ClassUniqueMessages[pclass][i];
                if (!StrLen(j)) { continue; }
                
                QuoteStorage[quoteCount++] = j;
            }
            
            if (!quoteCount) { logMsg = "Oh bugger there's no messages for this unique."; }
            else { logMsg = QuoteStorage[random(0, quoteCount-1)]; }
        }
        else
        {
            logMsg = ClassUniqueMessages[pclass][0];
            
            if (!StrLen(logMsg)) { logMsg = "Oh bugger there's no message for this unique."; } 
        }

        if (GetCVar("samsara_cl_printpickup")) { Print(s:logMsg); }
        else { Log(s:logMsg); }
    }
    
    if (soundmode == 1) { LocalAmbientSound(ClassUniqueSounds[pclass], 127); }
    else { ActivatorSound(ClassUniqueSounds[pclass], 127); }
    
    FadeRange(ClassFades[pclass][0], ClassFades[pclass][1], ClassFades[pclass][2], ClassFades[pclass][3],
    ClassFades[pclass][0], ClassFades[pclass][1], ClassFades[pclass][2], 0.0, itof(ClassFades[pclass][4]) / 35);
}

script SAMSARA_MARATHON (int class, int slot, int dropped)
{
    int giveboth    = isInvasion() || !(isCoop() || isSinglePlayer());
    int hasShotty   = CheckInventory("WSTE-M5 Combat Shotgun");
    int hasBoth     = CheckInventory("CanDualShotties");
    int limited     = CheckInventory("LevelLimiter");
    int limit       = GetCVar("sv_itemrespawn") || GetCVar("sv_weaponstay");
    int ammoFull    = CheckInventory("AmmoShell") >= (GetAmmoCapacity("AmmoShell") / ((dropped*3)+1));
    int i;
    // The above line is because of the quadupling of ammo capacity with dropped pickups
    // It's a really gross hack. I hate it. But it works.
    
    switch (slot)
    {
      case 1:
        GiveInventory("CanDualPistols", 1);
        GiveInventory("KKV-7 SMG Flechette", 1);
        break;
        
      case 3:
        if (limited && !dropped)
        {
            SetResultValue(0);
            terminate;
        }

        if ((dropped && hasShotty && ammoFull) ||
            (!dropped && (hasBoth || (hasShotty && limited)) && ammoFull)
           )
        {
            SetResultValue(0);
            terminate;
        }
        
        GiveInventory("Shell", 8 / (!!dropped+1));
        GiveInventory("AmmoShell", 8 / (!!dropped+1));
        GiveInventory("WSTE-M5 Combat Shotgun", 1);
        
        if (giveboth || (hasShotty && !dropped))
        {
            GiveInventory("CanDualShotties", 1);
        }
        
        if (limit)
        {
            GiveInventory("LevelLimiter", 1);
        }
        break;

      default:
        GiveInventory(ClassWeapons[class][slot][S_WEP], 1);

        if (ClassWeapons[class][slot][S_AMMO1] != "")
        {
            i = ammoCount(ClassWeapons[class][slot][S_AMMO1]);
            GiveInventory(ClassWeapons[class][slot][S_AMMO1], i*2);
        }

        if (ClassWeapons[class][slot][S_AMMO2] != "")
        {
            i = ammoCount(ClassWeapons[class][slot][S_AMMO2]);
            GiveInventory(ClassWeapons[class][slot][S_AMMO2], i*2);
        }
        break;
    }
}
