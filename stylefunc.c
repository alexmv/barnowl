#include "owl.h"

static const char fileIdent[] = "$Id$";

/* In all of these functions, 'fm' is expected to already be
 * initialized.
 */
   
void owl_stylefunc_basic(owl_fmtext *fm, owl_message *m)
{
  char *body, *indent, *ptr, *zsigbuff, frombuff[LINE];
  ZNotice_t *n;

  if (owl_message_is_type_zephyr(m) && owl_message_is_direction_in(m)) {
    n=owl_message_get_notice(m);

    /* get the body */
    body=owl_strdup(owl_message_get_body(m));
    body=realloc(body, strlen(body)+30);

    /* add a newline if we need to */
    if (body[0]!='\0' && body[strlen(body)-1]!='\n') {
      strcat(body, "\n");
    }
    
    /* do the indenting into indent */
    indent=owl_malloc(strlen(body)+owl_text_num_lines(body)*OWL_MSGTAB+10);
    owl_text_indent(indent, body, OWL_MSGTAB);
    
    /* edit the from addr for printing */
    strcpy(frombuff, owl_message_get_sender(m));
    ptr=strchr(frombuff, '@');
    if (ptr && !strncmp(ptr+1, ZGetRealm(), strlen(ZGetRealm()))) {
      *ptr='\0';
    }
    
    /* set the message for printing */
    owl_fmtext_append_normal(fm, OWL_TABSTR);
    
    if (owl_message_is_ping(m)) {
      owl_fmtext_append_bold(fm, "PING");
      owl_fmtext_append_normal(fm, " from ");
      owl_fmtext_append_bold(fm, frombuff);
      owl_fmtext_append_normal(fm, "\n");
    } else if (owl_message_is_loginout(m)) {
      char *ptr, host[LINE], tty[LINE];
      int len;
      
      ptr=owl_zephyr_get_field(n, 1, &len);
      strncpy(host, ptr, len);
      host[len]='\0';
      ptr=owl_zephyr_get_field(n, 3, &len);
      strncpy(tty, ptr, len);
      tty[len]='\0';

      if (owl_message_is_login(m)) {
	owl_fmtext_append_bold(fm, "LOGIN");
      } else if (owl_message_is_logout(m)) {
	owl_fmtext_append_bold(fm, "LOGOUT");
      }
      owl_fmtext_append_normal(fm, " for ");
      ptr=short_zuser(owl_message_get_instance(m));
      owl_fmtext_append_bold(fm, ptr);
      owl_free(ptr);
      owl_fmtext_append_normal(fm, " at ");
      owl_fmtext_append_normal(fm, host);
      owl_fmtext_append_normal(fm, " ");
      owl_fmtext_append_normal(fm, tty);
      owl_fmtext_append_normal(fm, "\n");
    } else {
      owl_fmtext_append_normal(fm, "From: ");
      if (strcasecmp(owl_message_get_class(m), "message")) {
	owl_fmtext_append_normal(fm, "Class ");
	owl_fmtext_append_normal(fm, owl_message_get_class(m));
	owl_fmtext_append_normal(fm, " / Instance ");
	owl_fmtext_append_normal(fm, owl_message_get_instance(m));
	owl_fmtext_append_normal(fm, " / ");
      }
      owl_fmtext_append_normal(fm, frombuff);
      if (strcasecmp(owl_message_get_realm(m), ZGetRealm())) {
	owl_fmtext_append_normal(fm, " {");
	owl_fmtext_append_normal(fm, owl_message_get_realm(m));
	owl_fmtext_append_normal(fm, "} ");
      }
      
      /* stick on the zsig */
      zsigbuff=owl_malloc(strlen(owl_message_get_zsig(m))+30);
      owl_message_pretty_zsig(m, zsigbuff);
      owl_fmtext_append_normal(fm, "    (");
      owl_fmtext_append_ztext(fm, zsigbuff);
      owl_fmtext_append_normal(fm, ")");
      owl_fmtext_append_normal(fm, "\n");
      owl_free(zsigbuff);
      
      /* then the indented message */
      owl_fmtext_append_ztext(fm, indent);
      
      /* make personal messages bold for smaat users */
      if (owl_global_is_userclue(&g, OWL_USERCLUE_CLASSES)) {
	if (owl_message_is_personal(m)) {
	  owl_fmtext_addattr((&m->fmtext), OWL_FMTEXT_ATTR_BOLD);
	}
      }
    }
    
    owl_free(body);
    owl_free(indent);
  } else if (owl_message_is_type_zephyr(m) && owl_message_is_direction_out(m)) {
    char *indent, *text, *zsigbuff, *foo;
      
    text=owl_message_get_body(m);
    
    indent=owl_malloc(strlen(text)+owl_text_num_lines(text)*OWL_MSGTAB+10);
    owl_text_indent(indent, text, OWL_MSGTAB);
    owl_fmtext_append_normal(fm, OWL_TABSTR);
    owl_fmtext_append_normal(fm, "To: ");
    foo=short_zuser(owl_message_get_recipient(m));
    owl_fmtext_append_normal(fm, foo);
    owl_free(foo);
    owl_fmtext_append_normal(fm, "  (Zsig: ");
    
    zsigbuff=owl_malloc(strlen(owl_message_get_zsig(m))+30);
    owl_message_pretty_zsig(m, zsigbuff);
    owl_fmtext_append_ztext(fm, zsigbuff);
    owl_free(zsigbuff);
    
    owl_fmtext_append_normal(fm, ")");
    owl_fmtext_append_normal(fm, "\n");
    owl_fmtext_append_ztext(fm, indent);
    if (text[strlen(text)-1]!='\n') {
      owl_fmtext_append_normal(fm, "\n");
    }
    
    owl_free(indent);
  } else if (owl_message_is_type_aim(m)) {
    char *indent;
    
    if (owl_message_is_loginout(m)) {
      owl_fmtext_append_normal(fm, OWL_TABSTR);
      if (owl_message_is_login(m)) {
	owl_fmtext_append_bold(fm, "AIM LOGIN");
      } else {
	owl_fmtext_append_bold(fm, "AIM LOGOUT");
      }
      owl_fmtext_append_normal(fm, " for ");
      owl_fmtext_append_normal(fm, owl_message_get_sender(m));
      owl_fmtext_append_normal(fm, "\n");
    } else if (owl_message_is_direction_in(m)) {
      indent=owl_malloc(strlen(owl_message_get_body(m))+owl_text_num_lines(owl_message_get_body(m))*OWL_MSGTAB+10);
      owl_text_indent(indent, owl_message_get_body(m), OWL_MSGTAB);
      owl_fmtext_append_bold(fm, OWL_TABSTR);
      owl_fmtext_append_bold(fm, "AIM from ");
      owl_fmtext_append_bold(fm, owl_message_get_sender(m));
      owl_fmtext_append_bold(fm, "\n");
      owl_fmtext_append_bold(fm, indent);
      if (indent[strlen(indent)-1]!='\n') {
	owl_fmtext_append_normal(fm, "\n");
      }
      owl_free(indent);
    } else if (owl_message_is_direction_out(m)) {
      indent=owl_malloc(strlen(owl_message_get_body(m))+owl_text_num_lines(owl_message_get_body(m))*OWL_MSGTAB+10);
      owl_text_indent(indent, owl_message_get_body(m), OWL_MSGTAB);
      owl_fmtext_append_normal(fm, OWL_TABSTR);
      owl_fmtext_append_normal(fm, "AIM sent to ");
      owl_fmtext_append_normal(fm, owl_message_get_recipient(m));
      owl_fmtext_append_normal(fm, "\n");
      owl_fmtext_append_ztext(fm, indent);
      if (indent[strlen(indent)-1]!='\n') {
	owl_fmtext_append_normal(fm, "\n");
      }
      owl_free(indent);
    }
  } else if (owl_message_is_type_admin(m)) {
    char *text, *header, *indent;
    
    text=owl_message_get_body(m);
    header=owl_message_get_attribute_value(m, "adminheader");
    
    indent=owl_malloc(strlen(text)+owl_text_num_lines(text)*OWL_MSGTAB+10);
    owl_text_indent(indent, text, OWL_MSGTAB);
    owl_fmtext_append_normal(fm, OWL_TABSTR);
    owl_fmtext_append_bold(fm, "OWL ADMIN ");
    owl_fmtext_append_ztext(fm, header);
    owl_fmtext_append_normal(fm, "\n");
    owl_fmtext_append_ztext(fm, indent);
    if (text[strlen(text)-1]!='\n') {
      owl_fmtext_append_normal(fm, "\n");
    }
    
    owl_free(indent);
  }
}

void owl_stylefunc_default(owl_fmtext *fm, owl_message *m)
{
  char *body, *indent, *ptr, *zsigbuff, frombuff[LINE];
  ZNotice_t *n;

  if (owl_message_is_type_zephyr(m) && owl_message_is_direction_in(m)) {
    n=owl_message_get_notice(m);
  
    /* get the body */
    body=owl_malloc(strlen(owl_message_get_body(m))+30);
    strcpy(body, owl_message_get_body(m));
    
    /* add a newline if we need to */
    if (body[0]!='\0' && body[strlen(body)-1]!='\n') {
      strcat(body, "\n");
    }
    
    /* do the indenting into indent */
    indent=owl_malloc(strlen(body)+owl_text_num_lines(body)*OWL_MSGTAB+10);
    owl_text_indent(indent, body, OWL_MSGTAB);
    
    /* edit the from addr for printing */
    strcpy(frombuff, owl_message_get_sender(m));
    ptr=strchr(frombuff, '@');
    if (ptr && !strncmp(ptr+1, ZGetRealm(), strlen(ZGetRealm()))) {
      *ptr='\0';
    }
    
    /* set the message for printing */
    owl_fmtext_append_normal(fm, OWL_TABSTR);
    
    if (owl_message_is_ping(m) && owl_message_is_private(m)) {
      owl_fmtext_append_bold(fm, "PING");
      owl_fmtext_append_normal(fm, " from ");
      owl_fmtext_append_bold(fm, frombuff);
      owl_fmtext_append_normal(fm, "\n");
    } else if (owl_message_is_loginout(m)) {
      char *ptr, host[LINE], tty[LINE];
      int len;
      
      ptr=owl_zephyr_get_field(n, 1, &len);
      strncpy(host, ptr, len);
      host[len]='\0';
      ptr=owl_zephyr_get_field(n, 3, &len);
      strncpy(tty, ptr, len);
      tty[len]='\0';
      
      if (owl_message_is_login(m)) {
	owl_fmtext_append_bold(fm, "LOGIN");
      } else if (owl_message_is_logout(m)) {
	owl_fmtext_append_bold(fm, "LOGOUT");
      }
      owl_fmtext_append_normal(fm, " for ");
      ptr=short_zuser(n->z_class_inst);
      owl_fmtext_append_bold(fm, ptr);
      owl_free(ptr);
      owl_fmtext_append_normal(fm, " at ");
      owl_fmtext_append_normal(fm, host);
      owl_fmtext_append_normal(fm, " ");
      owl_fmtext_append_normal(fm, tty);
      owl_fmtext_append_normal(fm, "\n");
    } else {
      owl_fmtext_append_normal(fm, owl_message_get_class(m));
      owl_fmtext_append_normal(fm, " / ");
      owl_fmtext_append_normal(fm, owl_message_get_instance(m));
      owl_fmtext_append_normal(fm, " / ");
      owl_fmtext_append_bold(fm, frombuff);
      if (strcasecmp(owl_message_get_realm(m), ZGetRealm())) {
	owl_fmtext_append_normal(fm, " {");
	owl_fmtext_append_normal(fm, owl_message_get_realm(m));
	owl_fmtext_append_normal(fm, "} ");
      }
      if (n->z_opcode[0]!='\0') {
	owl_fmtext_append_normal(fm, " [");
	owl_fmtext_append_normal(fm, owl_message_get_opcode(m));
	owl_fmtext_append_normal(fm, "] ");
      }
      
      /* stick on the zsig */
      zsigbuff=owl_malloc(strlen(owl_message_get_zsig(m))+30);
      owl_message_pretty_zsig(m, zsigbuff);
      owl_fmtext_append_normal(fm, "    (");
      owl_fmtext_append_ztext(fm, zsigbuff);
      owl_fmtext_append_normal(fm, ")");
      owl_fmtext_append_normal(fm, "\n");
      owl_free(zsigbuff);
      
      /* then the indented message */
      owl_fmtext_append_ztext(fm, indent);
      
      /* make private messages bold for smaat users */
      if (owl_global_is_userclue(&g, OWL_USERCLUE_CLASSES)) {
	if (owl_message_is_personal(m)) {
	  owl_fmtext_addattr((&m->fmtext), OWL_FMTEXT_ATTR_BOLD);
	}
      }
    }
    
    owl_free(body);
    owl_free(indent);
  } else if (owl_message_is_type_zephyr(m) && owl_message_is_direction_out(m)) {
    char *indent, *text, *zsigbuff, *foo;
    
    text=owl_message_get_body(m);
    
    indent=owl_malloc(strlen(text)+owl_text_num_lines(text)*OWL_MSGTAB+10);
    owl_text_indent(indent, text, OWL_MSGTAB);
    owl_fmtext_append_normal(fm, OWL_TABSTR);
    owl_fmtext_append_normal(fm, "Zephyr sent to ");
    foo=short_zuser(owl_message_get_recipient(m));
    owl_fmtext_append_normal(fm, foo);
    owl_free(foo);
    owl_fmtext_append_normal(fm, "  (Zsig: ");
    
    zsigbuff=owl_malloc(strlen(owl_message_get_zsig(m))+30);
    owl_message_pretty_zsig(m, zsigbuff);
    owl_fmtext_append_ztext(fm, zsigbuff);
    owl_free(zsigbuff);
    
    owl_fmtext_append_normal(fm, ")");
    owl_fmtext_append_normal(fm, "\n");
    owl_fmtext_append_ztext(fm, indent);
    if (text[strlen(text)-1]!='\n') {
      owl_fmtext_append_normal(fm, "\n");
    }
    
    owl_free(indent);
  } else if (owl_message_is_type_aim(m)) {
    char *indent;
    
    if (owl_message_is_loginout(m)) {
      owl_fmtext_append_normal(fm, OWL_TABSTR);
      if (owl_message_is_login(m)) {
	owl_fmtext_append_bold(fm, "AIM LOGIN");
      } else {
	owl_fmtext_append_bold(fm, "AIM LOGOUT");
      }
      owl_fmtext_append_normal(fm, " for ");
      owl_fmtext_append_normal(fm, owl_message_get_sender(m));
      owl_fmtext_append_normal(fm, "\n");
    } else if (owl_message_is_direction_in(m)) {
      indent=owl_malloc(strlen(owl_message_get_body(m))+owl_text_num_lines(owl_message_get_body(m))*OWL_MSGTAB+10);
      owl_text_indent(indent, owl_message_get_body(m), OWL_MSGTAB);
      owl_fmtext_append_bold(fm, OWL_TABSTR);
      owl_fmtext_append_bold(fm, "AIM from ");
      owl_fmtext_append_bold(fm, owl_message_get_sender(m));
      owl_fmtext_append_bold(fm, "\n");
      owl_fmtext_append_bold(fm, indent);
      if (indent[strlen(indent)-1]!='\n') {
	owl_fmtext_append_normal(fm, "\n");
      }
      owl_free(indent);
    } else if (owl_message_is_direction_out(m)) {
      indent=owl_malloc(strlen(owl_message_get_body(m))+owl_text_num_lines(owl_message_get_body(m))*OWL_MSGTAB+10);
      owl_text_indent(indent, owl_message_get_body(m), OWL_MSGTAB);
      owl_fmtext_append_normal(fm, OWL_TABSTR);
      owl_fmtext_append_normal(fm, "AIM sent to ");
      owl_fmtext_append_normal(fm, owl_message_get_recipient(m));
      owl_fmtext_append_normal(fm, "\n");
      owl_fmtext_append_ztext(fm, indent);
      if (indent[strlen(indent)-1]!='\n') {
	owl_fmtext_append_normal(fm, "\n");
      }
      owl_free(indent);
    }
  } else if (owl_message_is_type_admin(m)) {
    char *text, *header, *indent;
    
    text=owl_message_get_body(m);
    header=owl_message_get_attribute_value(m, "adminheader");
    
    indent=owl_malloc(strlen(text)+owl_text_num_lines(text)*OWL_MSGTAB+10);
    owl_text_indent(indent, text, OWL_MSGTAB);
    owl_fmtext_append_normal(fm, OWL_TABSTR);
    owl_fmtext_append_bold(fm, "OWL ADMIN ");
    owl_fmtext_append_ztext(fm, header);
    owl_fmtext_append_normal(fm, "\n");
    owl_fmtext_append_ztext(fm, indent);
    if (text[strlen(text)-1]!='\n') {
      owl_fmtext_append_normal(fm, "\n");
    }
    
    owl_free(indent);
  }
}

void owl_stylefunc_oneline(owl_fmtext *fm, owl_message *m)
{
  char *tmp;
  char *baseformat="%s %-13.13s %-11.11s %-12.12s ";
  char *sender, *recip;

  sender=short_zuser(owl_message_get_sender(m));
  recip=short_zuser(owl_message_get_recipient(m));
  
  if (owl_message_is_type_zephyr(m)) {
    owl_fmtext_append_spaces(fm, OWL_TAB);
    if (owl_message_is_login(m)) {
      tmp=owl_sprintf(baseformat, "<", "LOGIN", "", sender);
      owl_fmtext_append_normal(fm, tmp);
      owl_fmtext_append_normal(fm, "\n");
      owl_free(tmp);
    } else if (owl_message_is_logout(m)) {
      tmp=owl_sprintf(baseformat, "<", "LOGOUT", "", sender);
      owl_fmtext_append_normal(fm, tmp);
      owl_fmtext_append_normal(fm, "\n");
      owl_free(tmp);
    } else if (owl_message_is_ping(m)) {
      tmp=owl_sprintf(baseformat, "<", "PING", "", sender);
      owl_fmtext_append_normal(fm, tmp);
      owl_fmtext_append_normal(fm, "\n");
      owl_free(tmp);
    } else {
      if (owl_message_is_direction_in(m)) {
	tmp=owl_sprintf(baseformat, "<", owl_message_get_class(m), owl_message_get_instance(m), sender);
      } else if (owl_message_is_direction_out(m)) {
	tmp=owl_sprintf(baseformat, ">", owl_message_get_class(m), owl_message_get_instance(m), recip);
      } else {
	tmp=owl_sprintf(baseformat, "-", owl_message_get_class(m), owl_message_get_instance(m), sender);
      }
      owl_fmtext_append_normal(fm, tmp);
      if (tmp) owl_free(tmp);
      
      tmp=owl_strdup(owl_message_get_body(m));
      owl_util_tr(tmp, '\n', ' ');
      owl_fmtext_append_ztext(fm, tmp);
      owl_fmtext_append_normal(fm, "\n");
      if (tmp) owl_free(tmp);
    }
      
    /* make personal messages bold for smaat users */
    if (owl_global_is_userclue(&g, OWL_USERCLUE_CLASSES) &&
	owl_message_is_personal(m) &&
	owl_message_is_direction_in(m)) {
      owl_fmtext_addattr(fm, OWL_FMTEXT_ATTR_BOLD);
    }

    owl_free(sender);
    owl_free(recip);
    
  } else if (owl_message_is_type_aim(m)) {
    owl_fmtext_append_spaces(fm, OWL_TAB);
    if (owl_message_is_login(m)) {
      tmp=owl_sprintf(baseformat, "<", "AIM LOGIN", "", owl_message_get_sender(m));
      owl_fmtext_append_normal(fm, tmp);
      owl_fmtext_append_normal(fm, "\n");
      if (tmp) owl_free(tmp);
    } else if (owl_message_is_logout(m)) {
      tmp=owl_sprintf(baseformat, "<", "AIM LOGOUT", "", owl_message_get_sender(m));
      owl_fmtext_append_normal(fm, tmp);
      owl_fmtext_append_normal(fm, "\n");
      if (tmp) owl_free(tmp);
    } else {
      if (owl_message_is_direction_in(m)) {
	tmp=owl_sprintf(baseformat, "<", "AIM", "", owl_message_get_sender(m));
	owl_fmtext_append_normal(fm, tmp);
	if (tmp) owl_free(tmp);
      } else if (owl_message_is_direction_out(m)) {
	tmp=owl_sprintf(baseformat, ">", "AIM", "", owl_message_get_recipient(m));
	owl_fmtext_append_normal(fm, tmp);
	if (tmp) owl_free(tmp);
      }
      
      tmp=owl_strdup(owl_message_get_body(m));
      owl_util_tr(tmp, '\n', ' ');
      owl_fmtext_append_normal(fm, tmp);
      owl_fmtext_append_normal(fm, "\n");
      if (tmp) owl_free(tmp);

      /* make personal messages bold for smaat users */
      if (owl_global_is_userclue(&g, OWL_USERCLUE_CLASSES) && owl_message_is_direction_in(m)) {
	owl_fmtext_addattr(fm, OWL_FMTEXT_ATTR_BOLD);
      }
    }
  } else if (owl_message_is_type_admin(m)) {
    owl_fmtext_append_spaces(fm, OWL_TAB);
    owl_fmtext_append_normal(fm, "< ADMIN                                        ");
    
    tmp=owl_strdup(owl_message_get_body(m));
    owl_util_tr(tmp, '\n', ' ');
    owl_fmtext_append_normal(fm, tmp);
    owl_fmtext_append_normal(fm, "\n");
    if (tmp) owl_free(tmp);
  }

}