var rpcCall;
var currentUri;
var currentState = {"id": null , history: [], future: []};
var optionArr = {};
var remainingRefreshTimerStart = 30;
var remainingRefreshTimer = 30;
var timeSinceRefreshTimer = 0;
var statusColumnsPerRow = 10;

function debug(msg) {
    if (window.console && window.console.log)
        window.console.log('debug(cl): ' + msg);
}

// Escape and unescape <, &, ", and > for html
function htmlEscape(htmlString, escape) {
    if (escape) {
	htmlString = htmlString.replace(/\&/g, '&amp;');
	htmlString = htmlString.replace(/</g, '&lt;');
	htmlString = htmlString.replace(/>/g, '&gt;');
	htmlString = htmlString.replace(/"/g, '&quot;');
    }
    else {
 	htmlString = htmlString.replace(/\&amp;/g, '&');
	htmlString = htmlString.replace(/\&lt;/g, '<');
	htmlString = htmlString.replace(/\&gt;/g, '>');
	htmlString = htmlString.replace(/\&quot;/g, '"');
    }

    return htmlString;
}

// Keep the timer running 
function updateTimer() {
    $("#timerUTC").html((new Date()).toUTCString());
}

// Keep the remaining time running
function updateRemainingRefreshTimer() {
    timeSinceRefreshTimer += 1;
    if (remainingRefreshTimer == -1) {
	$("#remainingTimer").html(' (currently never refresh).' + '<br>' + timeSinceRefreshTimer + ' secs since last refresh.');
	setTimeout("updateRemainingRefreshTimer()", 1000);
	return;
    }

    if (remainingRefreshTimer > 0) {
	remainingRefreshTimer -= 1;
    }
    else if (remainingRefreshTimer == 0) {
	showContent();
	timeSinceRefreshTimer = 0;
    }
    $("#remainingTimer").html(' (' + remainingRefreshTimer + ' secs remaining).' + '<br>' + timeSinceRefreshTimer + ' secs since last refresh.');
    setTimeout("updateRemainingRefreshTimer()", 1000);
}

// Do when the document is ready
$(document).ready(function () {
    updateTimer();
    setInterval("updateTimer()", 500);
    updateRemainingRefreshTimer();
    rpcCall = $.jsonrpc({url:"/jsonrpc"});
    rpcCallAsync = $.jsonrpc({url:"/jsonrpc", async: true});

    // Start up the tabs at the top
    $("#header").tabs({
        collapsible: true,
        selected: -1
    });    
    // Start up the dialog box
    $("#dialogObjectInfo").dialog({ autoOpen: false,
				    buttons: { "Menu": function() {
					       }
					     },
				    position: ['right', $("#tree").offset().top],
				    height : 450,
				    width : 600
				  });
    // Start up the new value dialog
    $("#dialogNewValue").dialog({ autoOpen: false,
				  modal: true
				});
    // Start up the notication dialog
    $("#dialogNotify").dialog({ autoOpen: false,
				modal: true,
				title: 'Notification',
				buttons: { "Okay": function() {
					       $(this).dialog('close');
					   }
					 }
			      });

    // Used to set the current state
    currentUri = parseUri(document.location.href);
    reestablishState();
    refresh();

    showContent();
});

// If there is an ending slash, remove it
function chopSlash(path) {
    if ((path.charAt(path.length - 1) == '/') && (path.length > 1)) {
        return path.substring(0, path.length - 1);
    }
    return path;
}

// Get the children at the path chosen
function getTree(path, callback) {
    path = chopSlash(unescape(path));
    rpcCallAsync.zoo_get_children(path, function(data) {
        if ((path.length != 0) == (path.charAt(path.length - 1) != '/')) {
            path += '/';
        }
	var html = '<ul class="jqueryFileTree" style="display:none">';
        var needRefresh = false;
        var needExpand = null;
        if (data != null) {
            $.each(data, function(objType, val) {
                if (currentState.id == null) {
		    currentState.id = path + val;			
                    needRefresh = true;
                }
                if (currentState.id.indexOf(path + val + "/") == 0) {
                    needExpand = path + val + "/";
                }
                html += '<li class="directory collapsed"><a href="javascript:void(0)" rel="' + path + val + '/">' + val + '</a></li>';
            });
        }
        html += '</ul>';
        callback(html);
        if (needRefresh) {
            showContent();
        }
    });
}

function handleRPCerror(rpcObj, retObj) {
    if (rpcObj.error == true) {
	var msg = '<strong>Operation Failed:</strong><br>Error message: ' +
	    rpcObj.error_message;
	if (retObj === undefined ||
	    retObj === null) {
	    $("#dialogNotify").html(msg);
	}
	else {
	    $("#dialogNotify").html(msg + '<br>Output: ' + retObj);
	}
	$("#dialogNotify").dialog('option', 'title', 'Notification');
	$('#dialogNotify').dialog('open');
	return true;
    }
    else {
	return false;
    }
}

function handleError(retString) {
    var msg = '<strong>Error:</strong><br>Error message: ' + retString;
    $("#dialogNotify").html(msg);
    $('#dialogNotify').dialog('open');
}

function showNodeContent(elem) {
    var path = $(elem).attr('rel');
    path = chopSlash(path);
    // rel attribute is escaped, so unescape here
    goLevel(unescape(path));
}

// Get the parent from the id or -1 if no parent can be found (i.e. root)
function getParent(id) {
    var slashCount = id.match(/\//g);
    if (slashCount.length <= 1) {
	// Root
	return "/";
    }
    var lastIndex = id.lastIndexOf('/');
    if (lastIndex == -1) {
	return null;
    }
    id = id.slice(0, lastIndex);
    lastIndex = id.lastIndexOf('/');
    if (lastIndex == -1) {
	return null;
    }
    
    return id;
}

function sameId(id1, id2) {
    return JSON.stringify(id1) == JSON.stringify(id2);
}

// id should not be escaped, should be native
function goLevel(id) {
    if (id == null) {
        alert("goLevel: id is null.");
        return;
    }

    // If not the same as the currentState.id, add to history
    if (!sameId(currentState.id, id)) {
	currentState.history.push(id);
    }

    currentState.id = id;
    showContent();
}

// Tries to go back, but not guaranteed if no more history
function goBack() {
    // Can't do anything since the only entry is myself
    if (currentState.history.length <= 1) {
	return;
    }

    // Sanity check
    if (!sameId(currentState.history[currentState.history.length - 1], 
		currentState.id)) {
	alert("goBack: Current state id (" + currentState.id + 
	      ") and the last history id (" + 
	      currentState.history[currentState.history.length - 1] + 
	      ") are not the same!");
	return;
    }
    currentState.history.pop();

    // Add to the future
    currentState.future.push(currentState.id);

    currentState.id = currentState.history[currentState.history.length - 1];
    showContent();
}

function goForward() {
    if (currentState.future.length < 1) {
	return;
    }

    currentState.id = currentState.future.pop();
    currentState.history.push(currentState.id);
    showContent();
}

function refresh() {
    // Refresh tree
    $("#tree").html('');
    var filecb = function (elem) {
        showNodeContent(elem);
    };
    $("#tree").fileTree({
	expandSpeed: 250,
	collapseSpeed: 250,
        getTree: function (t, callback) {
            getTree(t, callback);
        }, 
       root : ''
    }, function (elem) {
        showNodeContent(elem);
    });

    if (currentState.id == null) {
        // Go to the root
    } else {
        // Otherwise, refresh the content
        showContent();
    }
}

function reestablishState() {
    if (typeof(currentUri.queryKey.state) == "undefined") {
        // no previous state
        return;
    }

    currentState.id = unescape(currentUri.queryKey.state);
}

// Refresh the path's children
function refreshNotifyableChildren(elem) {
    var eventObj = document.createEvent('MouseEvent');
    eventObj.initEvent('click', true, true);
    elem.dispatchEvent(eventObj);
    eventObj = document.createEvent('MouseEvent');
    eventObj.initEvent('click', true, true);
    elem.dispatchEvent(eventObj);
}

// Generate the html to display the refresh information
function refreshHtml()
{
    var htmlForRefresh = '<div id="refreshContent" style="width: 100%;">Refresh <a class="changeRefresh" id="now">now</a> or in <a class="changeRefresh" id ="10">10</a>, <a class="changeRefresh" id="30">30</a>, <a class="changeRefresh" id="60">60</a> seconds or <a class="changeRefresh" id="never">never</a><a id="remainingTimer"></a></div>';
    return htmlForRefresh;
}

// Main display
function showContent() {
    // Refresh the timer
    remainingRefreshTimer = remainingRefreshTimerStart;
    timeSinceRefreshTimer = 0;
    // Set the page link
    $("#pagelink").attr("href", currentUri.path + "?state=" + escape(currentState.id));

    // Get the actual object content from server
    if (currentState.id == null) {
	return;
    }
    var content = rpcCall.zoo_get(currentState.id);
    if (content == "") {
        content = "<empty>";
    }

    var html = '<table id="dialogTable" class="tablesorter text ui-corner-all"><thead><tr><th>Attribute </th><th>Value </th></tr></thead><tbody><tr><td><strong>data</strong></td><td>' + htmlEscape(content, true) + '</td></tr>'; 

    var buttons = {};
    buttons["Create znode"] = function(event) {
	$("#dialogNewValue").dialog('option', 'title', 'New child');
	var inputBoxsHtml = '<form><fieldset style="border:0;"><label for="name">Name</label><textarea style="width: 100%;" name="name" id="inputName" class="text ui-widget-content ui-corner-all" ></textarea><label for="value">Value</label><textarea style="width: 100%;" type="text" name="value" id="inputValue" class="text ui-widget-content ui-corner-all" ></textarea></fieldset></form>';
	$("#dialogNewValue").html(inputBoxsHtml);
	$("#dialogNewValue").dialog('open');

	// Set the buttons for creating
	var addButtons = {};
	addButtons["Enter"] = function(event) {
	    var createNode = currentState.id;
	    if (createNode.charAt(createNode.length - 1) != '/') {
		createNode += '/';
	    }
	    createNode += $("#inputName").val();
	    var ret = rpcCall.zoo_create(createNode,
					 $("#inputValue").val());
	    $("#dialogNewValue").dialog('close');
	    if (handleRPCerror(rpcCall, ret)) {
		return;
	    }
	    $("#dialogObjectInfo").dialog('close');
	    
	    // Refresh the children
	    var search = "#tree A[rel='" 
		+ escape(currentState.id) + "/']";
	    var elem = $(search).parent().get(0);
	    refreshNotifyableChildren(elem);
			    
	    // Show the new child
	    if (ret != null) {
		goLevel(ret);		
	    }
	    
	};
	addButtons["Cancel"] = function(event) {
	    $("#dialogNewValue").dialog('close');
	};
	$("#dialogNewValue").dialog('option', 'buttons', 
				    addButtons);
    };
    buttons["Edit znode"] = function(event) {
	$("#dialogNewValue").dialog('option', 'title', currentState.id);
	var inputBoxHtml = '<form><fieldset style="border:0;"><label for="newValue">New Value</label><textarea style="width:100%;" name="newValue" id="newValue" class="text ui-widget-content ui-corner-all" ></textarea></fieldset></form>';
        $("#dialogNewValue").html(inputBoxHtml);
//        $("#newValue").text($(this).parent().text());
        $("#newValue").text($('#dialogTable td:last').text());
	$("#dialogNewValue").dialog('open');

        $('textarea#newValue').autoResize({
            // On resize:
            onResize : function() {
   	        $(this).css({opacity:0.8});
	    },
            // After resize
            animateCallback : function() {
     	        $(this).css({opacity:1});
	    },
	    // Quite slow animation
            animateDuration : 300,
            extraSpace : 20
        });
	
        // Set the buttons
	var editButtons = {};
	editButtons["Enter"] = function(event) {
	    var ret = rpcCall.zoo_set(currentState.id, $("#newValue").val());
	    showContent();
	    $("#dialogNewValue").dialog('close');
            handleRPCerror(rpcCall, ret);
	};
	editButtons["Cancel"] = function(event) {
	    $("#dialogNewValue").dialog('close');
	};
	$("#dialogNewValue").dialog('option', 'buttons', 
				    editButtons);
    };
    buttons["Delete znode"] = function(event) {
	$("#dialogNewValue").dialog('option', 'title', 
				    event.originalEvent.target.textContent);
	$("#dialogNewValue").dialog('open');

	var inputBoxHtml = '<form><fieldset style="border:0;"><label for="newValue">Remove the children as well? </label><input type="checkbox" name="newValue" id="removeChildrenValue" class="text ui-corner-all"><br><br>Are you sure?</fieldset></form>';
	$("#dialogNewValue").html(inputBoxHtml);

	var removeButtons = {};
	removeButtons["Yes"] = function(event) {
	    var ret = rpcCall.zoo_delete(
		currentState.id, 
                $("#removeChildrenValue").attr('checked')); 
	    
	    $("#dialogNewValue").dialog('close');
	    if (handleRPCerror(rpcCall, ret)) {
		return;
	    }
	    $("#dialogObjectInfo").dialog('close');
	    
	    // Refresh the children
	    var search = "#tree A[rel='" + 
		escape(currentState.id) + "/']";
	    var elem = $(search).parent().parent().prev().parent().get(0);
	    if (elem != undefined) {
		refreshNotifyableChildren(elem);	
	    }
	    
	    // Show the parent information again
	    var parentId = getParent(currentState.id);
	    if (parentId != null) {
		goLevel(unescape(parentId));
	    }
	};
	removeButtons["No"] = function(event) {
	    $("#dialogNewValue").dialog('close');
	};
	$("#dialogNewValue").dialog('option', 'buttons', 
				    removeButtons);
    };
    html += '</tbody></table>';

    // Navigation buttons
    html += '<span id="navDiv" class="text ui-corner-all" style="float : left; width: 100%;"><div id="navButtons "style="float:left"> Navigation: ' + 
	'<a class="navButton" href="javascript:void(0)" rel="link">link</a>, ' +
	'<a class="navButton" href="javascript:void(0)" rel="parent">parent</a>, ' +
	'<a class="navButton" href="javascript:void(0)" rel="back">back</a>, ' +
	'<a class="navButton" href="javascript:void(0)" rel="forward">forward</a>' +
	'</div></span>';

    // Add more html for the changing the refreshing period
    html += refreshHtml();

    // Put the html into the dialogObjectInfo
    $("#dialogObjectInfo").html(html);

    // Handle the clicks of the "navButtons" events
    $(".navButton").click(function() {
	if ($(this).attr('rel') == "link") {
	    var linkHtml = htmlEscape(
		currentUri.protocol + '://'+ 
		    currentUri.host + ":" + currentUri.port +
		    currentUri.path + "?state=" + 
		    escape(currentState.id), true);
	    $("#dialogNotify").html(
		'<strong>Page link:</strong><br><a href="' + linkHtml +
		    '">' + linkHtml + '</a>');
	    $('#dialogNotify').dialog('open');
	}
	else if ($(this).attr('rel') == "parent") {
	    var parentId = getParent(currentState.id);
	    if (parentId != null) {
		goLevel(unescape(parentId));
	    }
        }
	else if ($(this).attr('rel') == "back") {
	    goBack();
        }
	else if ($(this).attr('rel') == "forward") {
	    goForward();
        }
    });

    // Handle the clicks of the "childAttribute" events
    $(".childAttribute").click(function() {
        showNodeContent($(this));
    });

    // Handle the clicks of "changeRefresh" events
    $(".changeRefresh").click(function() {
	if ($(this).attr('id') == "never") {
	    remainingRefreshTimerStart = -1;
	    remainingRefreshTimer = -1;
	}
        else if ($(this).attr('id') == "now") {
	    showContent();
        }
        else {
	    remainingRefreshTimerStart = $(this).attr('id');
	    remainingRefreshTimer = remainingRefreshTimerStart;
        }
    });

    $("#dialogObjectInfo").dialog(
	'option', 'title', '<strong>' + currentState.id + 
	'</strong>');

    // Show the dialog box
    $("#dialogObjectInfo").dialog("option", "buttons", buttons);
    $("#dialogObjectInfo").dialog('open');	
}

