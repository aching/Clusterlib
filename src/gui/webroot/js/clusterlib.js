var rpcCall;
var currentUri;
var currentState = {"id": null, history: [], future: []};
var optionArr = {};
var remainingRefreshTimerStart = 30;
var remainingRefreshTimer = 30;
var timeSinceRefreshTimer = 0;
var statusColumnsPerRow = 10;

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
    $("#timer").text(Date());
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
    // Used to set the current state
    currentUri = parseUri(document.location.href);
    reestablishState();
    refresh();
    // Start up the dialog box
    $("#dialogObjectInfo").dialog({ autoOpen: false,
				    buttons: { "Menu": function() {
					       }
					     },
				    position: ['right', $("#tree").offset().top],
				    height : 550,
				    width : 700
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
    rpcCallAsync.getNotifyableChildrenFromKey(path, function(data) {
        if (path.charAt(path.length - 1) != '/') {
            path += '/';
        }
	var html = '<ul class="jqueryFileTree" style="display:none">';
        var needRefresh = false;
        var needExpand = null;
        if (data != null) {
            $.each(data, function(objType, val) {
		for (var objIndex in val) {
		    // Note: rel with the id should always be escaped
                    html += '<li class="directory collapsed"><a href="javascript:void(0)" rel="' + escape(val[objIndex].id) + '/"><strong>' + htmlEscape(val[objIndex].type, true) + '</strong>&nbsp;&nbsp;-&nbsp;&nbsp;' + htmlEscape(val[objIndex].name, true) + '</a></li>';
                    for (var attr in val[objIndex]) {
                    }
		}

                if (currentState.id == null) {
		    currentState.id = val[objIndex].id;
                    needRefresh = true;
                }
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
    if (slashCount.length <= 3) {
	// Root
	return null;
    }
    var lastIndex = id.lastIndexOf('/');
    if (id == -1) {
	return null;
    }
    id = id.slice(0, lastIndex);
    lastIndex = id.lastIndexOf('/');
    if (id == -1) {
	return null;
    }
    id = id.slice(0, lastIndex);
    
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
       root : '/'
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

// Generate the html for notifyable specific information
function showApplications(applicationStatusArr) {
    if (applicationStatusArr === null ||
	applicationStatusArr === undefined ||
	applicationStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = applicationStatusArr;
    templateObj["name"] = 'Application(s)';
    $("#applications").setTemplateURL("js/status.tpl");
    $("#applications").processTemplate(templateObj);
}
function showGroups(groupStatusArr) {
    if (groupStatusArr === null ||
	groupStatusArr === undefined ||
	groupStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = groupStatusArr;
    templateObj["name"] = 'Group(s)';
    $("#groups").setTemplateURL("js/status.tpl");
    $("#groups").processTemplate(templateObj);
}
function showDataDistributions(dataDistributionStatusArr) {
    if (dataDistributionStatusArr === null ||
	dataDistributionStatusArr === undefined ||
	dataDistributionStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = dataDistributionStatusArr;
    templateObj["name"] = 'Data Distribution(s)';
    $("#dataDistributions").setTemplateURL("js/status.tpl");
    $("#dataDistributions").processTemplate(templateObj);
}
function showNodes(nodeStatusArr) {
    if (nodeStatusArr === null ||
	nodeStatusArr === undefined ||
	nodeStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = nodeStatusArr;
    templateObj["name"] = 'Node(s)';
    $("#nodes").setTemplateURL("js/status.tpl");
    $("#nodes").processTemplate(templateObj);
}
function showProcessSlots(processSlotStatusArr) {
    if (processSlotStatusArr === null ||
	processSlotStatusArr === undefined ||
	processSlotStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = processSlotStatusArr;
    templateObj["name"] = 'ProcessSlot(s)';
    $("#processSlots").setTemplateURL("js/status.tpl");
    $("#processSlots").processTemplate(templateObj);
}
function showPropertyListSlots(propertyListStatusArr) {
    if (propertyListStatusArr === null ||
	propertyListStatusArr === undefined ||
	propertyListStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = propertyListStatusArr;
    templateObj["name"] = 'Property List(s)';
    $("#propertyLists").setTemplateURL("js/status.tpl");
    $("#propertyLists").processTemplate(templateObj);
}
function showShards(shardStatusArr) {
    if (shardStatusArr === null ||
	shardStatusArr === undefined ||
	shardStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = shardStatusArr;
    templateObj["name"] = 'Shard(s)';
    $("#shards").setTemplateURL("js/status.tpl");
    $("#shards").processTemplate(templateObj);
}


function showQueueSlots(queueStatusArr) {
    if (queueStatusArr === null ||
	queueStatusArr === undefined ||
	queueStatusArr.length === 0) {
	return;
    }

    var templateObj = {};
    templateObj.table = queueStatusArr;
    templateObj["name"] = 'Queue(s)';
    $("#queues").setTemplateURL("js/status.tpl");
    $("#queues").processTemplate(templateObj);
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
    var content = rpcCall.getNotifyableAttributesFromKey(currentState.id);
    if (content === null) {
	handleError(
	    "Object: " + currentState.id + " likely does not exist anymore " + 
		"or the server is down, please refresh your page.");
	return;
    }

    var html = '<table id="dialogTable" class="tablesorter text ui-corner-all"><thead><tr><th>Attribute </th><th>Value </th></tr></thead><tbody><tr><td><strong>object key</strong></td><td>' + htmlEscape(content.id, true) + '</td></tr>'; 

    var addAttributeHtml = '';
    var buttons = {};
    for (var key in content) {
	if (key == "id" || key == "name" || key == "type" ||
	    key == "applicationSummary" || key == "groupSummary" || 
	    key == "dataDistributionSummary" || key == "nodeSummary" ||
            key == "processSlotSummary" || key == "propertyListSummary" ||
            key == "shardSummary" || key == "queueSummary") {
	    continue;
	}
	else if (key == "options") {
            optionArr = content[key].split("\b");
	    for (var i = 0; i < optionArr.length; i++) {
                // Remove a notifyable
		if (optionArr[i] == "Remove") {
		    buttons[optionArr[i]] = function(event) {
			$("#dialogNewValue").dialog('open');
			$("#dialogNewValue").dialog('option', 'title', 
			    event.originalEvent.target.textContent);
			$("#dialogNewValue").text(
			    "Are you sure?");
			// Set the buttons for removal
			var removeButtons = {};
			removeButtons["Yes"] = function(event) {
			    var ret = rpcCall.removeNotifyableFromKey(
				currentState.id); 

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
		}
                else if (optionArr[i] == "Start Process" ||
                         optionArr[i] == "Stop Process") {
		    buttons[optionArr[i]] = function(event) {
			$("#dialogNewValue").dialog('open');
			$("#dialogNewValue").dialog('option', 'title', 
			    event.originalEvent.target.textContent);
			$("#dialogNewValue").html(
			    "Are you sure?");
			// Set the buttons for notifyable change
			var removeButtons = {};
			removeButtons["Yes"] = function(event) {
	                    var setAttributesArr = Array();
	                    setAttributesArr.push(currentState.id);
	                    var titleString = $("#dialogNewValue").dialog('option', 'title');
                            setAttributesArr.push("");
	                    setAttributesArr.push(titleString);
                            setAttributesArr.push("");

	                    var ret = 
                                rpcCall.setNotifyableAttributesFromKey(
                                    setAttributesArr);
	                    showContent();
	                    $("#dialogNewValue").dialog('close');
	                    handleRPCerror(rpcCall, ret);
			};
			removeButtons["No"] = function(event) {
			    $("#dialogNewValue").dialog('close');
			};
			$("#dialogNewValue").dialog('option', 'buttons', 
						    removeButtons);
		    };
                    
                    
                }
		else {
		    buttons[optionArr[i]] = function(event) {
			$("#dialogNewValue").dialog('open');
			$("#dialogNewValue").dialog('option', 'title', event.originalEvent.target.textContent);
	                var inputBoxHtml = '<form><fieldset style="border:0;"><label for="newValue">Name</label><textarea style="width:100%;" name="newValue" id="newValue" class="text ui-corner-all" ></textarea></fieldset></form>';
			$("#dialogNewValue").html(inputBoxHtml);

			// Set the buttons for adding
			var addButtons = {};
			addButtons["Enter"] = function(event) {
			    var ret = rpcCall.addNotifyableFromKey(currentState.id, $("#dialogNewValue").dialog('option', 'title'), $("#newValue").val());

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
		}
	    }
	    continue;
	}
	else if (key == "addAttribute") {
	    // TODO: allow many add attributes
	    addAttributeHtml += '<a class=addAttribute rel="' + 
		htmlEscape(content[key], true) + 
		'" style="float:right"><img src="../images/plus.gif"/>' + 
		htmlEscape(content[key], true) + '</a><br>';
	    continue;
	}

	// Add each attribute (some are edittable or deletable)
	// if they are edittable or deletable, the key is the last part
	var editDeleteKeyArr = key.split(";");
	if (editDeleteKeyArr.length == 1) {
            html += '<tr><td><strong>' + htmlEscape(key, true) + '</strong></td><td>' + htmlEscape(JSON.stringify(content[key], true)) + '</td></tr>';
	}
	else {
            html += '<tr><td><strong>' + 
		htmlEscape(editDeleteKeyArr[editDeleteKeyArr.length - 1], true) + '</strong></td><td>' + htmlEscape(JSON.stringify(content[key]), true);
	    for (var i = 0; i < editDeleteKeyArr.length - 1; i++) {
		html += '<a class="' + htmlEscape(editDeleteKeyArr[i], true) +
		    'Attribute"><img src="../images/' + 
		    htmlEscape(editDeleteKeyArr[i], true) + 
		    '.gif" style="float:right"/></a>';
	    }
	    html += '</td></tr>';
	}
    }
    html += '</tbody></table>';
    html += addAttributeHtml;

    html += '<span id="statusDiv" class="text ui-corner-all" style="float : left; width: 100%;"><div id="applications"></div><div id="groups"></div><div id="dataDistributions"></div><div id="nodes\"></div><div id="processSlots"</div><div id="shards\"></div><div id="queues"</div><div id="propertyLists"></div></span>';

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

    // Based on the type, fill in the appropriate div
    var type = content.type;
    if (type == "Root") {
	showApplications(content["applicationSummary"]);
	showPropertyListSlots(content["propertyListSummary"]);
    }
    else if (type == "Application" || type == "Group") {
	showGroups(content["groupSummary"]);
	showDataDistributions(content["dataDistributionSummary"]);
	showNodes(content["nodeSummary"]);
        showQueueSlots(content["queueSummary"]);
	showPropertyListSlots(content["propertyListSummary"]);
    } 
    else if (type == "Node") {
	showProcessSlots(content["processSlotSummary"]);
	showQueueSlots(content["queueSummary"]);
	showPropertyListSlots(content["propertyListSummary"]);
    } 
    else if (type == "DataDistribution") {
        showShards(content["shardSummary"]);
	showQueueSlots(content["queueSummary"]);
	showPropertyListSlots(content["propertyListSummary"]);
    } 
    else if (type == "PropertyList") {
    } 
    else if (type == "ProcessSlot") {
	showQueueSlots(content["queueSummary"]);
	showPropertyListSlots(content["propertyListSummary"]);
    } 
    else if (type == "Queue") {
	showPropertyListSlots(content["propertyListSummary"]);
    }
    else {
        // Should never happen
        alert("Unknown type in showContent: " + type);
    }

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

    // Handle the clicks of "addAttribute" events
    $(".addAttribute").click(function() {
	$("#dialogNewValue").dialog('option', 'title', 
				    unescape($(this).text()));

        // Based on the text show different choices (value only) or
        // (name & value)
        var inputBoxsHtml;
        if ($(this).text() == "Add queue element") {
            inputBoxsHtml = '<form><fieldset style="border:0;"><label for="value">Value</label><textarea style="width: 100%;" type="text" name="value" id="inputValue" class="text ui-widget-content ui-corner-all" ></textarea></fieldset></form>';
        }
	else {
            inputBoxsHtml = '<form><fieldset style="border:0;"><label for="name">Name</label><textarea style="width: 100%;" name="name" id="inputName" class="text ui-widget-content ui-corner-all" ></textarea><label for="value">Value</label><textarea style="width: 100%;" type="text" name="value" id="inputValue" class="text ui-widget-content ui-corner-all" ></textarea></fieldset></form>';
        }

        $("#dialogNewValue").html(inputBoxsHtml);
        if ($('textarea#inputName').length) {
            $('textarea#inputName').autoResize({
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
        }
        if ($('textarea#inputValue').length) {
            $('textarea#inputValue').autoResize({
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
        }

	$("#dialogNewValue").dialog('open');
				  
        // Set the buttons
	var editButtons = {};
	editButtons["Enter"] = function(event) {
	    var addAttributesArr = Array();
	    addAttributesArr.push(currentState.id);
	    addAttributesArr.push(
		$("#dialogNewValue").dialog('option', 'title'));
	    addAttributesArr.push($("#inputName").val());
	    addAttributesArr.push($("#inputValue").val());
	    var ret = 
		rpcCall.setNotifyableAttributesFromKey(addAttributesArr);
	    showContent();
	    $("#dialogNewValue").dialog('close');
	    handleRPCerror(rpcCall, ret);
	};
	editButtons["Cancel"] = function(event) {
	    $("#dialogNewValue").dialog('close');
	};
	$("#dialogNewValue").dialog('option', 'buttons', 
				    editButtons);
    });

    // Handle the clicks of "editAttribute" events
    $(".editAttribute").click(function() {
	$("#dialogNewValue").dialog('option', 'title', 
				    unescape($(this).parent().prev().text()));
	var inputBoxHtml = '<form><fieldset style="border:0;"><label for="newValue">New Value</label><textarea style="width:100%;" name="newValue" id="newValue" class="text ui-widget-content ui-corner-all" ></textarea></fieldset></form>';
        $("#dialogNewValue").html(inputBoxHtml);
        $("#newValue").text($(this).parent().text());
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
	    var setAttributesArr = Array();
	    setAttributesArr.push(currentState.id);
	    var titleString = $("#dialogNewValue").dialog('option', 'title');
	    var splitIndex = titleString.indexOf(' ');
	    var op = titleString.substring(0, splitIndex);
	    var attributeName = titleString.substring(splitIndex + 1);
	    
	    setAttributesArr.push(op);
	    setAttributesArr.push(attributeName);
            var jsonObj;
            try {
                jsonObj = JSON.parse($("#newValue").val());                
            } catch (x) {
                handleError(
                    'Bad input syntax : ' + $("#newValue").val() + '<br><br>' + 'Possibly missing quotes (") around strings, closed brackets ([]) around arrays, or braces ({}) around objects.');
	        $("#dialogNewValue").dialog('close');
                return;
            }
	    setAttributesArr.push(jsonObj);
	    var ret = rpcCall.setNotifyableAttributesFromKey(setAttributesArr);
	    showContent();
	    $("#dialogNewValue").dialog('close');
            handleRPCerror(rpcCall, ret);
	};
	editButtons["Cancel"] = function(event) {
	    $("#dialogNewValue").dialog('close');
	};
	$("#dialogNewValue").dialog('option', 'buttons', 
				    editButtons);
    });

    // Handle the clicks of "deleteAttribute" events
    $(".deleteAttribute").click(function() {
	$("#dialogNewValue").dialog('option', 'title', 
				    unescape($(this).parent().prev().text()));
	var inputBoxHtml = 'Are you sure you want to delete?';
        $("#dialogNewValue").html(inputBoxHtml);
	$("#dialogNewValue").dialog('open');
				  
        // Set the buttons
	var editButtons = {};
	editButtons["Yes"] = function(event) {
	    var titleString = $("#dialogNewValue").dialog('option', 'title');
	    var splitIndex = titleString.indexOf(' ');
	    var op = titleString.substring(0, splitIndex);
	    var attributeName = titleString.substring(splitIndex + 1);

	    var opOtherArr = 
		$("#dialogNewValue").dialog('option', 'title').split(' ');

	    var ret = rpcCall.removeNotifyableAttributesFromKey(
		currentState.id,
		op,
		attributeName);
	    showContent();
	    $("#dialogNewValue").dialog('close');
	    handleRPCerror(rpcCall, ret);
	};
	editButtons["No"] = function(event) {
	    $("#dialogNewValue").dialog('close');
	};
	$("#dialogNewValue").dialog('option', 'buttons', 
				    editButtons);
    });

    // Use table sorter
    $("#dialogTable").tablesorter();
    $("#dialogObjectInfo").dialog(
	'option', 'title', '<strong>' + content.type + 
	'</strong>&nbsp;&nbsp;-&nbsp;&nbsp;' + htmlEscape(content.name, true));
    // Show the dialog box
    $("#dialogObjectInfo").dialog("option", "buttons", buttons);
    $("#dialogObjectInfo").dialog('open');	
}
