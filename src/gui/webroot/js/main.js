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
    // Used to set the current state
    currentUri = parseUri(document.location.href);
    reestablishState();
    refresh();
    setInterval("refresh()", 60*1000*5);
});

function idType(id) {
    if (id == null || typeof(id.type) == "undefined") {
        return "N/A";
    } else if (id.type == "node") {
        return "Node";
    } else if (id.type == "group") {
        return "Group";
    } else if (id.type == "dataDistribution") {
        return "Distribution";
    } else if (id.type == "properties") {
        return "Properties";
    } else {
        return "Application";
    }
}

function idName(id) {
    if (id == null || typeof(id.name) == "undefined") {
        return "N/A";
    } else {
        return id.name;
    }
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
    return JSON.stringify(id1.id) == JSON.stringify(id2.id);
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

function goApplication(id) {
    currentState.id = id;
    currentState.upid = [];
    showContent();
}

function refresh() {
    // Get all application list
    var apps = rpcCall.getApplications();
    var status = rpcCall.getApplicationStatus(apps);
    $("#applist").empty();
    $.each(apps, function (key, val) {
        // Add the name to the application list
        var lielem = $("<li/>");
        var aelem = $("<a/>").attr("href", "javascript:void(0)").addClass(status[key]).text(idName(val)).data("id", val);
        lielem.append(aelem).appendTo("#applist");
    });
    $("#applist li a").click(function () {
        goApplication($(this).data("id"));
    });
    if (currentState.id == null) {
        // When no application is selected, show the first application
        if (apps.length > 0) {
            goApplication(apps[0]);
        }
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

    currentState.id = JSON.parse(unescape(currentUri.queryKey.state));
}

function showContent() {
    // Set the page link
    $("#pagelink").attr("href", currentUri.path + "?state=" + escape(JSON.stringify(currentState.id)));
    // Set the property title
    $("#propertycaption").html(idType(currentState.id) + ": <b><i>" + idName(currentState.id) + "</i></b>");
    // Clear existing content
    $("#propertylist").empty();
    if ($("#graphbox").data("graph")) {
        $("#graphbox").data("graph").clear();
        $("#graphbox").removeData("graph");
    }
    $("#graphbox").empty();
    var type = idType(currentState.id);
    if (type == "Application") {
        showApplication();
    } else if (type == "Group") {
        showGroup();
    } else if (type == "Node") {
        showNode();
    } else if (type == "Distribution") {
        showDistribution();
    } else if (type == "Properties") {
        showPropertiesObjects();
    } else {
        // Should never happen
        alert("Unknown type " + type);
    }
}

function generatePropertyRow(key, val) {
    var rowElem = $("<div/>").addClass("row");
    var keyElem = $("<div/>").addClass("key").append(key);
    if (typeof(val) == "string" && val == "") {
        val = "&nbsp;"
    }
    var valueElem = $("<div/>").addClass("cell").append(val);
    rowElem.append(keyElem);
    rowElem.append(valueElem);
    return rowElem;
}

function showProperties(obj) {
    // Add name as the first property
    generatePropertyRow("Name", idName(currentState.id)).appendTo("#propertylist");
    var statusRow;
    if (obj != null) {
        statusRow = generatePropertyRow("Status", obj.status);
        statusRow.children(".cell").addClass(obj.status);
    } else {
        statusRow = generatePropertyRow("Status", "Error");
        statusRow.children(".cell").addClass("Bad");
    }

    statusRow.appendTo("#propertylist");

    if (obj == null) {
	alert("Null object");
        return;
    }

    $.each(obj.properties, function (key,val) {
        // Show custom properties
        generatePropertyRow(key, val).appendTo("#propertylist");
    });
    
    if (obj.parent) {
        generatePropertyRow("Parent " + idType(obj.parent).toLowerCase(), $("<a/>").attr("href", "javascript:void(0)").data("id", obj.parent).text(idName(obj.parent)).click(function(){
            goLevel($(this).data("id"));
        })).appendTo("#propertylist");
    }
}

function showApplication() {
    var app = rpcCall.getApplication(currentState.id);
    showProperties(app);
    if (app == null) return;
    $("<div/>").addClass("graph").append($("<canvas/>").attr("id", "graph")).appendTo("#graphbox");
    $("<div/>").attr("id", "groupbox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").attr("id", "distribbox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").attr("id", "nodebox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").attr("id", "propertiesbox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").addClass("caption").text("Group status summary").appendTo("#groupbox");
    $("<div/>").addClass("caption").text("Data distribution status summary").appendTo("#distribbox");
    $("<div/>").addClass("caption").text("Node summary").appendTo("#nodebox");
    $("<div/>").addClass("caption").text("Properties summary").appendTo("#propertiesbox");

    var groupStatus = rpcCall.getGroupStatus(app.groups);
    var distribStatus = rpcCall.getDataDistributionStatus(app.dataDistributions);
    var nodeStatus = rpcCall.getNodeStatus(app.nodes);

    aggGroupStatus = [0, 0, 0];
    aggDistribStatus = [0, 0, 0];
    aggNodeStatus = [0, 0, 0];

    // List groups
    $.each(app.groups, function (key,val) {
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass(groupStatus[key]).appendTo("#groupbox");
        if (groupStatus[key] == "Good") {
            ++aggGroupStatus[0];
        } else if (groupStatus[key] == "Warning") {
            ++aggGroupStatus[1];
        } else if (groupStatus[key] == "Bad") {
            ++aggGroupStatus[2];
        }
    });
    // List distributions
    $.each(app.dataDistributions, function (key,val) {
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass(distribStatus[key]).appendTo("#distribbox");
        if (distribStatus[key] == "Good") {
            ++aggDistribStatus[0];
        } else if (distribStatus[key] == "Warning") {
            ++aggDistribStatus[1];
        } else if (distribStatus[key] == "Bad") {
            ++aggDistribStatus[2];
        }
    });
    // List nodes
    $.each(app.nodes, function (key,val) {
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass(nodeStatus[key]).appendTo("#nodebox");
        if (nodeStatus[key] == "Good") {
            ++aggNodeStatus[0];
        } else if (nodeStatus[key] == "Warning") {
            ++aggNodeStatus[1];
        } else if (nodeStatus[key] == "Bad") {
            ++aggNodeStatus[2];
        }
    });
    // List properties
    $.each(app.propertiesObjects, function (key,val) {
//        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass(propertiesStatus[key]).appendTo("#propertiesbox");
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass("Good").appendTo("#propertiesbox");
    });

    $("#groupbox a").click(function() {
        goLevel($(this).data("id"));
    });

    $("#distribbox a").click(function() {
        goLevel($(this).data("id"));
    });

    $("#nodebox a").click(function() {
        goLevel($(this).data("id"));
    });

    $("#propertiesbox a").click(function() {
        goLevel($(this).data("id"));
    });

    // Add bar graph
    if (window.G_vmlCanvasManager) {
        G_vmlCanvasManager.initElement(document.getElementById("graph"));
    }
    var g = new Bluff.Bar("graph", "400x300");
    g.title = "Status";
    g.theme_37signals();
    g.data("Group", aggGroupStatus, 'green');
    g.data("Data Distribution", aggDistribStatus, 'blue');
    g.data("Node", aggNodeStatus, 'purple');
    g.labels = {0: "Good", 1:"Warning", 2:"Bad"};
    g.minimum_value = 0;
    g.draw();
    $("#graphbox").data("graph", g);
}

function showGroup() {
    var group = rpcCall.getGroup(currentState.id);
    showProperties(group);
    if (group == null) return;
    
    // Show group-specific properties
    generatePropertyRow("Is leader", group.isLeader ? "Yes" : "No" ).appendTo("#propertylist");

    $("<div/>").addClass("graph").append($("<canvas/>").attr("id", "graph")).appendTo("#graphbox");
    $("<div/>").attr("id", "groupbox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").attr("id", "distribbox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").attr("id", "nodebox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").attr("id", "propertiesbox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").addClass("caption").text("Group status summary").appendTo("#groupbox");
    $("<div/>").addClass("caption").text("Data distribution status summary").appendTo("#distribbox");
    $("<div/>").addClass("caption").text("Node summary").appendTo("#nodebox");
    $("<div/>").addClass("caption").text("Properties summary").appendTo("#propertiesbox");

    var groupStatus = rpcCall.getGroupStatus(group.groups);
    var distribStatus = rpcCall.getDataDistributionStatus(group.dataDistributions);
    var nodeStatus = rpcCall.getNodeStatus(group.nodes);

    aggGroupStatus = [0, 0, 0];
    aggDistribStatus = [0, 0, 0];
    aggNodeStatus = [0, 0, 0];

    // List groups
    $.each(group.groups, function (key,val) {
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass(groupStatus[key]).appendTo("#groupbox");
        if (groupStatus[key] == "Good") {
            ++aggGroupStatus[0];
        } else if (groupStatus[key] == "Warning") {
            ++aggGroupStatus[1];
        } else if (groupStatus[key] == "Bad") {
            ++aggGroupStatus[2];
        }
    });
    // List distributions
    $.each(group.dataDistributions, function (key,val) {
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass(distribStatus[key]).appendTo("#distribbox");
        if (distribStatus[key] == "Good") {
            ++aggDistribStatus[0];
        } else if (distribStatus[key] == "Warning") {
            ++aggDistribStatus[1];
        } else if (distribStatus[key] == "Bad") {
            ++aggDistribStatus[2];
        }
    });
    // List nodes
    $.each(group.nodes, function (key,val) {
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val)).data("id",val)).addClass(nodeStatus[key]).appendTo("#nodebox");
        if (nodeStatus[key] == "Good") {
            ++aggNodeStatus[0];
        } else if (nodeStatus[key] == "Warning") {
            ++aggNodeStatus[1];
        } else if (nodeStatus[key] == "Bad") {
            ++aggNodeStatus[2];
        }
    });

    $("#groupbox a").click(function() {
        goLevel($(this).data("id"));
    });

    $("#distribbox a").click(function() {
        goLevel($(this).data("id"));
    });

    $("#nodebox a").click(function() {
        goLevel($(this).data("id"));
    });

    $("#propertiesbox a").click(function() {
        goLevel($(this).data("id"));
    });

    // Add bar graph
    if (window.G_vmlCanvasManager) {
        G_vmlCanvasManager.initElement(document.getElementById("graph"));
    }
    var g = new Bluff.Bar("graph", "400x300");
    g.title = "Status";
    g.theme_37signals();
    g.data("Group", aggGroupStatus, 'green');
    g.data("Data Distribution", aggDistribStatus, 'blue');
    g.data("Node", aggNodeStatus, 'purple');
    g.labels = {0: "Good", 1:"Warning", 2:"Bad"};
    g.minimum_value = 0;
    g.draw();
    $("#graphbox").data("graph", g);
}

function showNode() {
    var node = rpcCall.getNode(currentState.id);
    showProperties(node);
    if (node == null) return;
    
    // Show node-specific properties
    generatePropertyRow("Is healthy", node.isHealthy ? "Yes" : "No" ).appendTo("#propertylist");
    generatePropertyRow("Is connected", node.isConnected ? "Yes" : "No" ).appendTo("#propertylist");
    generatePropertyRow("State", node.state).appendTo("#propertylist");
    generatePropertyRow("Master state", node.masterState).appendTo("#propertylist");
    var date = new Date();
    date.setTime(node.lastStateTime);
    generatePropertyRow("State report", date.toString()).appendTo("#propertylist");
    date.setTime(node.lastMasterStateTime);
    generatePropertyRow("Master state report", date.toString()).appendTo("#propertylist");
    date.setTime(node.lastConnectionTime);
    generatePropertyRow("Connection report", date.toString()).appendTo("#propertylist");
}

function showDistribution() {
    var distrib = rpcCall.getDataDistribution(currentState.id);
    showProperties(distrib);
    if (distrib == null) return;
    
    // Show distribution-specific properties
    if (!distrib.isCovered) {
        // Not covered, fatal status
        var rowelem = generatePropertyRow("Is covered", "No");
        rowelem.children(".cell").addClass("Bad");
        rowelem.appendTo("#propertylist");
    } else {
        generatePropertyRow("Is covered", "Yes").appendTo("#propertylist");
    }

    generatePropertyRow("Number of shards", distrib.shards.length).appendTo("#propertylist");
    
    $("<div/>").addClass("graph").append($("<canvas/>").attr("id", "graph")).appendTo("#graphbox");
    $("<div/>").attr("id", "shardbox").addClass("fullbox").appendTo("#graphbox");
    $("<div/>").addClass("caption").text("Shard status summary").appendTo("#shardbox");

    var nodes = [];
    var distribs = [];

    $.each(distrib.shards, function(key,val) {
        if (val.isForward) {
            distribs.push(val.id);
        } else {
            nodes.push(val.id);
        }
/*
        generatePropertyRow("Shard " + key, $("<span/>").text(val.low + " to " + val.high + ", served by ").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val.id)).data("id", val.id).click(function() {
            goLevel($(this).data("id"));
        }))).appendTo("#propertylist");
*/
    });

    var nodeStatus = rpcCall.getNodeStatus(nodes);
    var distribStatus = rpcCall.getDataDistributionStatus(distribs);
    var shardStatus = [];

    $.each(distrib.shards, function(key,val) {
        if (val.isForward) {
            shardStatus.push(distribStatus.shift());
        } else {
            shardStatus.push(nodeStatus.shift());
        }
    });

    aggShardStatus = [0, 0, 0];

    // List shards
    $.each(distrib.shards, function (key,val) {
        $("<div/>").addClass("itembox").append($("<a/>").attr("href", "javascript:void(0)").text(idName(val.id)).data("id",val.id).addClass("info").append(
            $("<span/>").text("Serving hash " + val.low + " to " + val.high + (val.isFoward ? " by fowarding" : ""))
        )).addClass(shardStatus[key]).appendTo("#shardbox");
        // This should be changed to other node-related status
        if (shardStatus[key] == "Good") {
            ++aggShardStatus[0];
        } else if (shardStatus[key] == "Warning") {
            ++aggShardStatus[1];
        } else if (shardStatus[key] == "Bad") {
            ++aggShardStatus[2];
        }
    });

    $("#shardbox a").click(function() {
        goLevel($(this).data("id"));
    });

    // Add bar graph
    if (window.G_vmlCanvasManager) {
        G_vmlCanvasManager.initElement(document.getElementById("graph"));
    }
    var g = new Bluff.Bar("graph", "400x300");
    g.title = "Status";
    g.theme_37signals();
    g.data("Shard", aggShardStatus, 'blue');
    g.labels = {0: "Good", 1:"Warning", 2:"Bad"};
    g.minimum_value = 0;
    g.draw();
    $("#graphbox").data("graph", g);
}

function showPropertiesObjects() {
    var propertiesObject = rpcCall.getProperties(currentState.id);
    showProperties(propertiesObject);
    if (propertiesObject == null) return;
}

