var rpcCall;
var currentUri;
var currentState = {"id": null, upid: []};

function updateTimer() {
    $("#timer").text(Date());
}

$(document).ready(function () {
    $("#uplink").click(function () {
        goUpLevel();
    });
    $("#refresh").click(function () {
        refresh();
    });
    updateTimer();
    setInterval("updateTimer()", 500);
    rpcCall = $.jsonrpc({url:"/jsonrpc"});
    rpcCallAsync = $.jsonrpc({url:"/jsonrpc", async: true});
    // Used to set the current state
    currentUri = parseUri(document.location.href);
    reestablishState();
    refresh();
    setInterval("refresh()", 60*1000*5);
    $("#nodecontent").click(function() {
	setContent();
    });
});

function chopSlash(path) {
    if ((path.charAt(path.length - 1) == '/') && (path.length > 1)) {
        return path.substring(0, path.length - 1);
    }
    return path;
}

function getTree(path, callback) {
    path = chopSlash(unescape(path));
    rpcCallAsync.zoo_get_children(path, function(data) {
        if (path.charAt(path.length - 1) != '/') path += '/';
        var html = '<ul class="jqueryFileTree" style="display:none">';
        var needRefresh = false;
        var needExpand = null;
        if (data != null) {
            $.each(data, function(key, val) {
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
        if (needExpand != null) {
            $("#tree A[rel='" + needExpand + "']").parent().click();
        }
    });
}

function showNodeContent(elem) {
    var path = $(elem).attr('rel');
    path = chopSlash(path);
    goLevel(path);
}

function goUpLevel() {
    if (currentState == null || currentState.upid.length == 0) {
        // No up level to go
        return;
    }

    currentState.id = currentState.upid.pop();
    showContent();
}

function sameId(id1, id2) {
    return JSON.stringify(id1) == JSON.stringify(id2);
}

function goLevel(id) {
    if (id == null) {
        alert("It is no available.");
        return;
    }
    var popping = false;
    for (var i = 0; i < currentState.upid.length; ++i) {
        if (sameId(currentState.upid[i], id)) {
            while (currentState.upid.length > i) currentState.upid.pop();
            popping = true;
            break;
        }
    }
    if (!popping) currentState.upid.push(currentState.id);
    currentState.id = id;
    showContent();
}

function refresh() {
    // Refresh tree
    $("#tree").html('');
    var filecb = function (elem) {
        showNodeContent(elem);
    }
    $("#tree").fileTree({
	expandSpeed: -1,
	collapseSpeed: -1,
        getTree: function (t, callback) {
            getTree(t, callback);
        }, root : '/'
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

function showContent() {
    // Set the page link
    $("#pagelink").attr("href", currentUri.path + "?state=" + escape(currentState.id));

    // Get the actual object content from server
    var content = rpcCall.zoo_get(currentState.id);
    if (content == "") {
        content = "<empty>";
    }
    $("#nodecontent").text(content);
    $("#propertycaption").text("Content (" + currentState.id + ")");
}

function setContent() {
    var newValue = prompt("Enter the new value of " + currentState.id, "<empty>");
    if (newValue != '' && newValue != null) {
	if (newValue == "<empty>") {
	    newValue = "";
        }
        var content = rpcCall.zoo_set(currentState.id, newValue);
        if (content != 0) {
            alert("Set of " + currentState.id + " to " + newValue + " failed with error " + content);
        }
    }

    showContent();
}
