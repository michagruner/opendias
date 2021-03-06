$(document).ready(function () {

  setLoginOutArea();

  $('#loginform').bind('submit', $('#loginform'), function(e) { 
    var form = this;
    e.preventDefault();
    e.stopPropagation();
    if (form.submitted) { return; }
    form.submitted = true;
    attemptLogin(form);
  });

  $('#logoutbutton').click(function () {
    $('#password').val('');
    $.ajax({
      url: "/opendias/dynamic",
      dataType: "xml",
      timeout: AJAX_TIMEOUT,
      data: {
        action: "logout"
      },
      cache: false,
      type: "POST",
      error: function (x, t, m) {
        if (t == "timeout") {
          alert("[y001] " + LOCAL_timeout_talking_to_server);
        } else {
          alert("[y002] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
        }
      },
      success: function (data) {
        if ($(data).find('error').text()) {
          alert($(data).find('error').text());
        } else {
          deleteCookie("realname", null);
          document.location.href = "/opendias/";
        }
      }
    });
  });

  showBubble();

});


function attemptLogin(form) {
  $('#loginbutton').attr("disabled", true);
  $.ajax({
    url: "/opendias/dynamic",
    dataType: "xml",
    timeout: AJAX_TIMEOUT,
    data: {
      action: "checkLogin",
      username: $('#username').val(),
      password: $('#password').val(),
    },
    cache: false,
    type: "POST",
    error: function (x, t, m) {
      $('#password').val('');
      form.submitted = false;
      if (t == "timeout") {
        alert("[y003] " + LOCAL_timeout_talking_to_server);
      } else {
        alert("[y004] " + LOCAL_error_talking_to_server + ": " + t + "\n" + m);
      }
    },
    success: function (data) {
      $('#password').val('');
      if ($(data).find('error').text()) {
        alert($(data).find('error').text());
        form.submitted = false;
      } else {
        if ($(data).find('result').text() == 'OK') {
          $('#loginbutton').attr("disabled", false);
          form.submitted = false;
          form.submit(); //invoke the save password in browser - which happily reloads the page.
        } else {
          $('#loginbutton').css({
            display: 'none'
          });
          setTimeout(function () {
            $('#loginbutton').attr("disabled", false);
            $('#loginbutton').css({
              display: 'inline'
            });
          }, parseInt($(data).find('retry_throttle').text()) * 1000);
          alert($(data).find('message').text());
          form.submitted = false;
        }
      }
    }
  });
}

function setLoginOutArea() {
  // Display or not, the login area
  // If we have a cookie "realname=<anything>"
  // then show the logout only. Otherwise
  // show the login area.
  var realname = getCookie("realname");
  if (realname == null || realname == "") {
    if ( window.location.pathname != '/opendias/') {
      // Not logged in and not on the homepage.
      document.location.href='/opendias/';
    }
    // Not logged in, but on the homepage
    $('#logout').css({
      display: 'none'
    });
    $('#login').css({
      display: 'block'
    });
  } else {
    // Logged in
    $('#realname').html(realname);
    $('#login').css({
      display: 'none'
    });
    $('#logout').css({
      display: 'block'
    });
  }
  updateMenuLinks();
}

function updateMenuLinks() {

  // Remove links the user can't use
  var role = getCookie("role");
  if ( ! get_priv_from_role(role, 'view_doc')) {
    $('#doclistLink').parent().css({
      display: 'none'
    });
  }
  if ( ! ( get_priv_from_role(role, 'add_import') || get_priv_from_role(role, 'add_scan') ) ) {
    $('#acquireLink').parent().css({
      display: 'none'
    });
  }
}


// This does not form security, it just stop
// presenting forms and calling API function, that 
// we know are going to casue a 'permission denied'
// response.

function get_priv_from_role(user_role, priv) {

  if (user_role == undefined) {
    return 0;
  }

  // These details are set in the application using the 'access_role' table.
  //         | update_access | view_doc | edit_doc | delete_doc | add_import | add_scan 
  // 1,"admin",   1,              1,          1,        1,            1,          1
  // 2,"user",    0,              1,          1,        1,            1,          1
  // 3,"view",    0,              1,          0,        0,            0,          0
  // 4,"add",     0,              0,          0,        0,            1,          1

  var role = [];
  role[1] = {};
  role[2] = {};
  role[3] = {};
  role[4] = {};

  role[1].name = LOCAL_role_admin;
  role[2].name = LOCAL_role_user;
  role[3].name = LOCAL_role_view;
  role[4].name = LOCAL_role_add;

  role[1].update_access = 1;
  role[2].update_access = 0;
  role[3].update_access = 0;
  role[4].update_access = 0;

  role[1].view_doc = 1;
  role[2].view_doc = 1;
  role[3].view_doc = 1;
  role[4].view_doc = 0;

  role[1].edit_doc = 1;
  role[2].edit_doc = 1;
  role[3].edit_doc = 0;
  role[4].edit_doc = 0;

  role[1].delete_doc = 1;
  role[2].delete_doc = 1;
  role[3].delete_doc = 0;
  role[4].delete_doc = 0;

  role[1].add_import = 1;
  role[2].add_import = 1;
  role[3].add_import = 0;
  role[4].add_import = 1;

  role[1].add_scan = 1;
  role[2].add_scan = 1;
  role[3].add_scan = 0;
  role[4].add_scan = 1;

  return role[user_role][priv];
}

function showBubble() {

  var newProgress = "";
  var progress = getCookie("progress");

  if (progress == null && getCookie("realname") != null ) {
    progress = "shown_login_help";
    setCookie("progress", progress);
  }

  if (progress == null) {
    $('#infoBubble').css({ display: 'block' });
    $('#message').html("<h3>" + LOCAL_howto_login_title + "</h3>"
                      + "<p>" + LOCAL_howto_login + "</p>"
                      + "<p>" + LOCAL_login_firsttime + "</p>");
    newProgress = "shown_login_help";
  }

  else if( progress == "shown_login_help" && getCookie("realname") != null ) {
    $('#infoBubble').css({ display: 'block' });
    $('#infoBubble').css({ width: '180px' });
    $('#message').html("<h3>" + LOCAL_user_admin_title + "</h3>"
                      + "<p>" + LOCAL_user_admin + "</p>");
    newProgress = "shown_user_admin";
  }

  $('#infoBubble #dismiss').click( function() {
    $('#infoBubble').css({ display: 'none' });
    if( newProgress != "" ) {
      setCookie("progress", newProgress);
      showBubble();
    }
  });

}

