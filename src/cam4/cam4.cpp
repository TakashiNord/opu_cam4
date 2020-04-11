//////////////////////////////////////////////////////////////////////////////
//
//  cam4.cpp
//
//  Description:
//      Contains Unigraphics entry points for the application.
//
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE 1

//  Include files
#include <uf.h>
#include <uf_exit.h>
#include <uf_ui.h>

#include <uf_obj.h>

#include <uf_ui_ont.h>

#include <uf_oper.h>
#include <uf_path.h>

#include <uf_part.h>
#include <uf_param.h>
#include <uf_param_indices.h>

/*
#if ! defined ( __hp9000s800 ) && ! defined ( __sgi ) && ! defined ( __sun )
# include <strstream>
  using std::ostrstream;
  using std::endl;
  using std::ends;
#else
# include <strstream.h>
#endif
#include <iostream.h>
*/

#include "cam4.h"

#include <stdio.h>

#define UF_CALL(X) (report( __FILE__, __LINE__, #X, (X)))

static int report( char *file, int line, char *call, int irc)
{
  if (irc)
  {
     char    messg[133];
     printf("%s, line %d:  %s\n", file, line, call);
     (UF_get_fail_message(irc, messg)) ?
       printf("    returned a %d\n", irc) :
       printf("    returned error %d:  %s\n", irc, messg);
  }
  return(irc);
}

int do_cam4();

//----------------------------------------------------------------------------
//  Activation Methods
//----------------------------------------------------------------------------

//  Explicit Activation
//      This entry point is used to activate the application explicitly, as in
//      "File->Execute UG/Open->User Function..."
extern "C" DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    int errorCode = UF_initialize();

    if ( 0 == errorCode )
    {
        /* TODO: Add your application code here */
        do_cam4();

        /* Terminate the API environment */
        errorCode = UF_terminate();
    }

    /* Print out any error messages */
    PrintErrorMessage( errorCode );
}

//----------------------------------------------------------------------------
//  Utilities
//----------------------------------------------------------------------------

// Unload Handler
//     This function specifies when to unload your application from Unigraphics.
//     If your application registers a callback (from a MenuScript item or a
//     User Defined Object for example), this function MUST return
//     "UF_UNLOAD_UG_TERMINATE".
extern "C" int ufusr_ask_unload( void )
{
     /* unload immediately after application exits*/
    return ( UF_UNLOAD_IMMEDIATELY );

     /*via the unload selection dialog... */
     //return ( UF_UNLOAD_SEL_DIALOG );
     /*when UG terminates...              */
     //return ( UF_UNLOAD_UG_TERMINATE );
}

/* PrintErrorMessage
**
**     Prints error messages to standard error and the Unigraphics status
**     line. */
static void PrintErrorMessage( int errorCode )
{
    if ( 0 != errorCode )
    {
        /* Retrieve the associated error message */
        char message[133];
        UF_get_fail_message( errorCode, message );

        /* Print out the message */
        UF_UI_set_status( message );

        fprintf( stderr, "%s\n", message );
    }
}


/*****************************************************************************************/

int do_cam4()
{
/*  Variable Declarations */
    char str[133];
    char menu_main[14][38] ;
    char menu[10][16] ; double  da[10];
    double tool_offset;

    tag_t       tls = NULL_TAG;
    tag_t       prg = NULL_TAG;
    int   i , count = 0 ;
    int   obj_count = 0;
    tag_t *tags = NULL ;
    char  prog_name[UF_OPER_MAX_NAME_LEN+1];
    int type, subtype ;
    int resp ;

    logical rsp;
    UF_UDE_t *ude_objs; int num_of_udes;
    UF_UDE_t  ude_obj;  char *ude_name;
    //int p;  int number_of_params; char **param_names;
    int origin_flag;
    int k;
    // UF_PATH_id_t path_id; UF_OPER_id_t oper_id;
    // double origin_coordinates[3]; char text[]={""};

    int cnt_org=0,cnt_tls=0,cnt_opr=0;

    int need_generate;
    logical generated;
    int generat;

    int response1 , response2 ;
    char *mes1[]={
      "Программа предназначена для установки параметров для угловой головки в операции.",
      "Для этого,Вы должны :",
      "  1) выбрать необходимые операции и нажать кнопку 'Далее..'",
      "  2) в появившемся окне установить необходимые опции ",
      " ",
      NULL
    };
    UF_UI_message_buttons_t buttons1 = { TRUE, FALSE, TRUE, "Далее....", NULL, "Отмена", 1, 0, 2 };
    char *mes2[]={
      "Производить генерацию операции после изменения?",
      NULL
    };
    UF_UI_message_buttons_t buttons2 = { TRUE, FALSE, TRUE, "Генерировать..", NULL, "Нет", 1, 0, 2 };

/*
    response1=0;
    UF_UI_message_dialog("Cam.....", UF_UI_MESSAGE_INFORMATION, mes1, 5, TRUE, &buttons1, &response1);
    if (response1!=1) { return (0) ; }
*/

    int module_id;
    UF_ask_application_module(&module_id);
    if (UF_APP_CAM!=module_id) {
       // UF_APP_GATEWAY UF_APP_CAM UF_APP_MODELING UF_APP_NONE
       uc1601("Запуск DLL - производится из модуля обработки\n - 2005г.",1);
       return (-1);
    }

    /* Ask displayed part tag */
    if (NULL_TAG==UF_PART_ask_display_part()) {
      uc1601("Cam-часть не активна.....\n программа прервана.",1);
      return (0);
    }

 /********************************************************************************/

  printf("\n\ndo_cam4()\n\n");

  strcpy(&menu_main[0][0], "Set -> Z-offset,Start UDE's=(ORIGIN)\0");
  strcpy(&menu_main[1][0], "Del -> Z-offset,Start UDE's=(ORIGIN)\0");
  strcpy(&menu_main[2][0], "Help\0");

  response1=3;
  while (response1 != 1 && response1 != 2  && response1 != 19 )
  {

   response1 = uc1603("Select operations for UDE's & Z-offset to create - Back or Cancel to terminate or finish.",1,menu_main, 2+1);

   if (response1 >= 3 || response1 < 19 )
   {
       switch (response1)
       {
          case 5 : // Set

      /********************************************************************************/

      // инициализация массива
      for(i=0;i<10;i++) { da[i]=25.; }

      da[0]=25.;//tool
      da[1]=25.;//oper
      da[2]=25.;//x
      da[3]=0.0;//y
      da[4]=0.0;//z
      /********************************************/
      strcpy(&menu[0][0], "Tool Y/Z Offset\0");
      strcpy(&menu[1][0], "Oper Z Offset=\0");
      strcpy(&menu[2][0], "ORIGIN/x=\0");
      strcpy(&menu[3][0], "ORIGIN/y=\0");
      strcpy(&menu[4][0], "ORIGIN/z=\0");

      response2 = uc1609("..Параметры Offset..", menu, 5, da, &i);
      if (response2 != 3 && response2 != 4) { break; }

      /* Get the number of selected operation objects. */
      if (obj_count>0) { obj_count=0 ; UF_free(tags); }
      UF_CALL( UF_UI_ONT_ask_selected_nodes(&obj_count, &tags) );
      if (obj_count<=0) { uc1601("Не выбрано операций!\n..",1); break ; }

      generat=1;
      UF_UI_message_dialog("Cam.....", UF_UI_MESSAGE_QUESTION, mes2, 1, TRUE, &buttons2, &generat);
      if (generat==2) { generat=0; }

      UF_UI_toggle_stoplight(1);

      for(i=0,count=0,cnt_org=0,cnt_tls=0,cnt_opr=0;i<obj_count;i++)
      {
        prg = tags[i]; // идентификатор объекта

        need_generate=0;

        UF_CALL( UF_OBJ_ask_type_and_subtype (prg, &type, &subtype ) );
        if (type!=UF_machining_operation_type) { continue ; }

        prog_name[0]='\0';
        //UF_OBJ_ask_name(prg, prog_name);// спросим имя обьекта
        UF_OPER_ask_name_from_tag(prg, prog_name);
        printf("\n Set prog_name =%s ",prog_name);

        /*******************************************************/
        UF_OPER_ask_cutter_group(prg,&tls);
        if (tls!=null_tag)
        {
         /* UF_PARAM_TL_Z_OFFSET - UF_PARAM_TL_Z_OFFSET_TOG
          - UF_PARAM_TL_X_OFFSET - UF_PARAM_TL_X_OFFSET_TOG
          - UF_PARAM_TL_Y_OFFSET - UF_PARAM_TL_Y_OFFSET_TOG */
         UF_CALL( UF_PARAM_ask_double_value(tls,UF_PARAM_TL_Z_OFFSET,&tool_offset));
         if (tool_offset!=da[0]) {
           tool_offset=da[0];
           UF_CALL( UF_PARAM_set_double_value(tls,UF_PARAM_TL_Z_OFFSET,tool_offset));
           resp=UF_CALL( UF_PARAM_set_double_value(tls,UF_PARAM_TL_Y_OFFSET,tool_offset));
           cnt_tls++;
           need_generate++;
         }
        }

        /*******************************************************/
        UF_CALL( UF_PARAM_ask_int_value(prg,UF_PARAM_TL_Z_OFFSET_TOG,&resp));
        if (resp==1) {
           UF_CALL( UF_PARAM_ask_double_value(prg,UF_PARAM_TL_Z_OFFSET,&tool_offset));
           if (tool_offset!=da[1]) {
              UF_CALL( UF_PARAM_set_double_value(prg,UF_PARAM_TL_Z_OFFSET,-9999.));
              tool_offset=da[1];
              UF_CALL( UF_PARAM_set_double_value(prg,UF_PARAM_TL_Z_OFFSET,tool_offset));
              cnt_opr++;
              need_generate++;
           }
        } else {
           UF_CALL( UF_PARAM_set_int_value(prg,UF_PARAM_TL_Z_OFFSET_TOG,1));
           UF_CALL( UF_PARAM_set_double_value(prg,UF_PARAM_TL_Z_OFFSET,-9999.));
           tool_offset=da[1];
           UF_CALL( UF_PARAM_set_double_value(prg,UF_PARAM_TL_Z_OFFSET,tool_offset));
           cnt_opr++;
           need_generate++;
        }

        /*******************************************************/
        origin_flag=0;

        /* данный блок работает если ORIGIN(s) - уже существует  */
        UF_CALL( UF_PARAM_ask_udes(prg, UF_UDE_START_SET, &num_of_udes, &ude_objs ) );
        if (num_of_udes!=0) {
         for(k=0;k<num_of_udes;k++)
         {
          ude_obj=ude_objs[k];
          UF_CALL( UF_UDE_ask_name(ude_obj,&ude_name) );
          printf(" %d) ude_name=%s",k,ude_name);
          if (0==strcmp(ude_name,"origin")) {
            /*UF_UDE_ask_params(ude_obj,&number_of_params, &param_names );
            for(p=0;p<number_of_params;p++)
            {
              printf("\t %d) %s",p,param_names[p]);
            }
            UF_free_string_array(number_of_params,param_names);*/
            origin_flag++;
            UF_CALL( UF_UDE_set_double(ude_obj,"X",da[2]) );
            UF_CALL( UF_UDE_set_double(ude_obj,"Y",da[3]) );
            UF_CALL( UF_UDE_set_double(ude_obj,"Z",da[4]) );
            cnt_org++;
            need_generate++;
            UF_free(ude_name); break ; // !!!!!!! Важно меняем токо первое ORIGIN - если оно есть !!!!!!!!
          }
          UF_free(ude_name);
         }
         UF_free(ude_objs);
         num_of_udes=0;
        }


        if (origin_flag==0) {
           //ude_obj=(void *)null_tag;
           // !!!!!!!! Важно !!!!!!!! названия UDE - должны быть маленькими буквами
           UF_CALL(UF_PARAM_can_accept_ude(prg, UF_UDE_START_SET, "origin", &rsp));
           if (rsp) {
             UF_CALL(UF_PARAM_append_ude(prg,UF_UDE_START_SET,"origin",&ude_obj));
             //UF_CALL(UF_UDE_set_param_toggle(ude_obj,"command_status",UF_UDE_PARAM_ACTIVE));
             UF_CALL(UF_UDE_set_double(ude_obj,"X",da[2]));
             UF_CALL(UF_UDE_set_double(ude_obj,"Y",da[3]));
             UF_CALL(UF_UDE_set_double(ude_obj,"Z",da[4]));
             UF_CALL(UF_UDE_set_string(ude_obj,"origin_text",""));
             cnt_org++;
             need_generate++;
             origin_flag++;
           }
        }

        UF_CALL(UF_PARAM_can_accept_ude_set(prg,UF_UDE_START_SET,&rsp));
        if (!rsp) {
          str[0]='\0';
          sprintf(str,"В операции %s \n START User Defined Event - не активировано ",prog_name);
          uc1601(str,1);
        }

        /*
        oper_id = (UF_OPER_id_t) prg;
        UF_CALL( resp=UF_OPER_ask_path(oper_id,&path_id) );
        if (resp==0) {
           UF_CALL( UF_PATH_init_tool_path( path_id ) );
           UF_CALL( UF_PATH_create_origin(path_id,origin_coordinates,text) );
           UF_CALL( UF_PATH_end_tool_path( path_id ) );
        }
        */

        if (generat==1 && need_generate>0) { UF_CALL( UF_PARAM_generate (prg,&generated ) ); }
        count++;

      }

      if (obj_count>0) { obj_count=0 ; UF_free(tags); }

      UF_UI_ONT_refresh();
      UF_UI_toggle_stoplight(0);

      str[0]='\0'; sprintf(str,"Change parameters :\nObject(s)=%d of %d\nInsert Start UDE's (ORIGIN)=%d\nTool(s)=%d",cnt_opr,count,cnt_org,cnt_tls);
      uc1601(str,1);

          break ;
          case 6 : // Delete

      /********************************************************************************/
      /* Get the number of selected operation objects. */
      if (obj_count>0) { obj_count=0 ; UF_free(tags); }
      UF_CALL( UF_UI_ONT_ask_selected_nodes(&obj_count, &tags) );
      if (obj_count<=0) { uc1601("Не выбрано операций!\n..",1); break ; }

      generat=1;
      UF_UI_message_dialog("Cam.....", UF_UI_MESSAGE_QUESTION, mes2, 1, TRUE, &buttons2, &generat);
      if (generat==2) { generat=0; }

      UF_UI_toggle_stoplight(1);

      for(i=0,count=0,cnt_org=0,cnt_tls=0,cnt_opr=0;i<obj_count;i++)
      {
         prg = tags[i]; // идентификатор объекта

         need_generate=0;

         UF_CALL( UF_OBJ_ask_type_and_subtype (prg, &type, &subtype ) );
         if (type!=UF_machining_operation_type) { continue ; }

         prog_name[0]='\0';
         //UF_OBJ_ask_name(prg, prog_name);// спросим имя обьекта
         UF_OPER_ask_name_from_tag(prg, prog_name);
         printf("\n Del prog_name =%s ",prog_name);

         /*******************************************************/
         UF_CALL(UF_PARAM_ask_udes(prg, UF_UDE_START_SET, &num_of_udes, &ude_objs ));
         if (num_of_udes!=0) {
           printf("\n Del num_of_udes =%d ",num_of_udes);
           for(k=0;k<num_of_udes;k++)
           {
            ude_obj=ude_objs[k];
            UF_CALL(UF_UDE_ask_name(ude_obj,&ude_name));
            if (0==strcmp(ude_name,"origin")) {
              UF_CALL( UF_PARAM_delete_ude(prg, UF_UDE_START_SET, ude_obj) );
              printf("\n\t Del origin =%d ",k);
              cnt_org++;
              need_generate++;
            }
            UF_free(ude_name);
           }
           UF_free(ude_objs);
           num_of_udes=0;
         }

        /*******************************************************/
        UF_OPER_ask_cutter_group(prg,&tls);
        if (tls!=null_tag) {
         /* UF_PARAM_TL_Z_OFFSET - UF_PARAM_TL_Z_OFFSET_TOG
          - UF_PARAM_TL_X_OFFSET - UF_PARAM_TL_X_OFFSET_TOG
          - UF_PARAM_TL_Y_OFFSET - UF_PARAM_TL_Y_OFFSET_TOG */
         UF_CALL( UF_PARAM_ask_double_value(tls,UF_PARAM_TL_Z_OFFSET,&tool_offset) );
         if (tool_offset!=0.0) {
              UF_CALL( UF_PARAM_set_double_value(tls,UF_PARAM_TL_Z_OFFSET,-9999) );
              UF_CALL( UF_PARAM_set_double_value(tls,UF_PARAM_TL_Z_OFFSET,0.0) );
              cnt_tls++;
              need_generate++;
         }
         resp=0;
         resp=UF_CALL( UF_PARAM_ask_double_value(tls,UF_PARAM_TL_Y_OFFSET,&tool_offset) );
         if (tool_offset!=0.0 && resp==0) {
              UF_CALL( UF_PARAM_set_double_value(tls,UF_PARAM_TL_Y_OFFSET,-9999) );
              UF_CALL( UF_PARAM_set_double_value(tls,UF_PARAM_TL_Y_OFFSET,0.0) );
         }
        }

        /*******************************************************/
        resp=0;
        UF_CALL( UF_PARAM_ask_int_value(prg,UF_PARAM_TL_Z_OFFSET_TOG,&resp) );
        if (resp) {
          UF_CALL( UF_PARAM_ask_double_value(prg,UF_PARAM_TL_Z_OFFSET,&tool_offset) );
          if (tool_offset!=0.) {
            UF_CALL( UF_PARAM_set_double_value(prg,UF_PARAM_TL_Z_OFFSET,-9999.) );
            UF_CALL( UF_PARAM_set_double_value(prg,UF_PARAM_TL_Z_OFFSET,0.0) );
          }
          UF_CALL( UF_PARAM_set_int_value(prg,UF_PARAM_TL_Z_OFFSET_TOG,0) );
          cnt_opr++;
          need_generate++;
        }

        if (generat==1 && need_generate>0) { UF_CALL( UF_PARAM_generate (prg,&generated ) ); }
        count++;

      }

      if (obj_count>0) { obj_count=0 ; UF_free(tags); }

      UF_UI_ONT_refresh();
      UF_UI_toggle_stoplight(0);

      str[0]='\0'; sprintf(str,"Delete parameters from\nObject(s)=%d of %d\nDelete Start UDE's (ORIGIN)=%d\nTool(s)=%d",cnt_opr,count,cnt_org,cnt_tls);
      uc1601(str,1);
      /********************************************************************************/

          break ;
          case 7 :
            UF_UI_open_listing_window();
            UF_UI_write_listing_window("\n#============================================================");
            UF_UI_write_listing_window("\n Автор:");
            UF_UI_write_listing_window("\n\t ЧЕ.");
            UF_UI_write_listing_window("\n#============================================================");
            UF_UI_write_listing_window("\n");
            UF_UI_write_listing_window("\n Программа предназначена для изменения Z-offset и Start User Defined событий в операциях.");
            UF_UI_write_listing_window("\n  1) Изменяет параметры операции и инструмента, а также вставляет событие:");
            UF_UI_write_listing_window("\n\t---------- ПП команды задаваемые пользователем в начале траектории ----------");
            UF_UI_write_listing_window("\n\tOrigin/Status=Active,X=25.000000,Y=0.000000,Z=0.000000,A_axis=0.000000,C_axis=0.000000");
            UF_UI_write_listing_window("\n  2) Может удалять назначенные параметры и User Defined события из операции.");
            UF_UI_write_listing_window("\n");
            UF_UI_write_listing_window("\n#=============================================================");
            UF_UI_write_listing_window("\nКак пользоваться:");
            UF_UI_write_listing_window("\n I) Установка параметров и User Defined события ORIGIN/ :");
            UF_UI_write_listing_window("\n    1) нажмите 'Set Z-offset,Start UDE's=(ORIGIN)'");
            UF_UI_write_listing_window("\n    2) задайте параметры: ");
            UF_UI_write_listing_window("\n            Tool Y/Z Offset=25 - данное значение вноситься в параметры инструмента");
            UF_UI_write_listing_window("\n            Oper Z Offset=25   - это значение вноситься в операцию");
            UF_UI_write_listing_window("\n            ORIGIN/x=25        - параметр UDE ORIGIN/");
            UF_UI_write_listing_window("\n            ORIGIN/y=0");
            UF_UI_write_listing_window("\n            ORIGIN/z=0");
            UF_UI_write_listing_window("\n    3) Выделите необходимые операции и нажмите 'Ok': ");
            UF_UI_write_listing_window("\n\n II) Удаление параметров и User Defined события ORIGIN/ :");
            UF_UI_write_listing_window("\n    1) Выделите необходимые операции и нажмите 'Del -> Z-offset,Start UDE's=(ORIGIN)': ");
            UF_UI_write_listing_window("\n       программа удалит параметры инструмента z-offset   ");
            UF_UI_write_listing_window("\n                                  операции    z-offset   ");
            UF_UI_write_listing_window("\n                        и User Defined событие ORIGIN/   ");
            UF_UI_write_listing_window("\n");
            UF_UI_write_listing_window("\n#============================================================\n$$");
          //UF_UI_close_listing_window () ;
          break ;
          default : break ;
        }
   }

  } // end while uc1603

 //UF_DISP_refresh ();

 printf("\n======================== end_of_program\n");

 return (0);
}




