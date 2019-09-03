static int testJoystick=1;

if (testJoystick==1)
{
   testJoystick=0;
   fprintf(log_get(),"%i joysticks were found.\n\n", SDL_NumJoysticks() );
   fprintf(log_get(),"The names of the joysticks are:\n");

   for(int tti=0; tti < SDL_NumJoysticks(); tti++ )
   {
      fprintf(log_get(),"    %s\n", SDL_JoystickName(tti));
   }

   SDL_JoystickEventState(SDL_ENABLE);
   joystick = SDL_JoystickOpen(0);
}
else
{
   if (joystick!=NULL)
   {
      SDL_JoystickClose(0);
      SDL_JoystickEventState(SDL_ENABLE);
      joystick = SDL_JoystickOpen(0);
   }
}

while ( SDL_PollEvent(&app_input_event) )
{
   if ( app_input_event.type == SDL_QUIT )
   {
      ws_key_esc = 1;
   }
}

if (joystick)
{
   if (SDL_JoystickGetButton(joystick,0))
   {
      ws_key_start=1;
   }
   else
   {
      ws_key_start=0;
   }

   if (SDL_JoystickGetButton(joystick,1))
   {
      ws_key_button_1=1;
   }
   else
   {
      ws_key_button_1=0;
   }

   if (SDL_JoystickGetButton(joystick,2))
   {
      ws_key_button_2=1;
   }
   else
   {
      ws_key_button_2=0;
   }


   if (SDL_JoystickGetAxis(joystick,0)<-7000)
   {
      ws_key_left=1;
   }
   else
   {
      ws_key_left=0;
   }

   if (SDL_JoystickGetAxis(joystick,0)>7000)
   {
      ws_key_right=1;
   }
   else
   {
      ws_key_right=0;
   }

   if (SDL_JoystickGetAxis(joystick,1)<-7000)
   {
      ws_key_up=1;
   }
   else
   {
      ws_key_up=0;
   }

   if (SDL_JoystickGetAxis(joystick,1)>7000)
   {
      ws_key_down=1;
   }
   else
   {
      ws_key_down=0;
   }
}
else
{
   ws_key_start=0;
   ws_key_left=0;
   ws_key_right=0;
   ws_key_up=0;
   ws_key_down=0;
   ws_key_button_1=0;
   ws_key_button_2=0;
}

uint8 *keystate = SDL_GetKeyState(NULL);

if ( keystate[SDLK_d])
{
   dump_memory();
}

if ( keystate[SDLK_r])
{
   printf("Boop\n");
   ws_reset();
}

if ( keystate[SDLK_ESCAPE] )
{
   ws_key_esc = 1;
}

if ( keystate[SDLK_UP] )
{
   ws_key_up=1;
}

if ( keystate[SDLK_DOWN] )
{
   ws_key_down=1;
}

if ( keystate[SDLK_RIGHT] )
{
   ws_key_right=1;
}

if ( keystate[SDLK_LEFT] )
{
   ws_key_left=1;
}

if (keystate[SDLK_RETURN])
{
   ws_key_start=1;
}

if (keystate[SDLK_c])
{
   ws_key_button_1=1;
}

if (keystate[SDLK_x])
{
   ws_key_button_2=1;
}

if (keystate[SDLK_p])
{
   ws_cyclesByLine+=10;
}

if (keystate[SDLK_o])
{
   ws_cyclesByLine-=10;
}

