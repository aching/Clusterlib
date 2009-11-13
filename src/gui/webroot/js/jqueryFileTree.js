// jQuery File Tree Plugin
//
// Version 1.01
//
// Cory S.N. LaViska
// A Beautiful Site (http://abeautifulsite.net/)
// 24 March 2008
//
// Visit http://abeautifulsite.net/notebook.php?article=58 for more information
//
// Usage: $('.fileTreeDemo').fileTree( options, callback )
//
// Options:  root           - root folder to display; default = /
//           script         - location of the serverside AJAX file to use; default = jqueryFileTree.php
//           folderEvent    - event to trigger expand/collapse; default = click
//           expandSpeed    - default = 500 (ms); use -1 for no animation
//           collapseSpeed  - default = 500 (ms); use -1 for no animation
//           expandEasing   - easing function to use on expand (optional)
//           collapseEasing - easing function to use on collapse (optional)
//           multiFolder    - whether or not to limit the browser to one subfolder at a time
//           loadMessage    - Message to display while initial tree loads (can be HTML)
//
// History:
//
// 1.01 - updated to work with foreign characters in directory/file names (12 April 2008)
// 1.00 - released (24 March 2008)
//
// TERMS OF USE
// 
// jQuery File Tree is licensed under a Creative Commons License and is copyrighted (C)2008 by Cory S.N. LaViska.
// For details, visit http://creativecommons.org/licenses/by/3.0/us/
//
function knackerEvent(eventObject) {
    if (!eventObject) {
        eventObject = window.event;
    }
    if (eventObject && eventObject.stopPropagation) {
        eventObject.stopPropagation();
    } else if (eventObject) {
        eventObject.cancelBubble = true;
    }
    if (eventObject && eventObject.preventDefault) {
        eventObject.preventDefault();
    } else if (eventObject) {
        eventObject.returnValue = false;
    }
}

// jquery bind is too slow, so we add events natively in js
function bindEventHandler(elem, eventName, handler) {
    // Firefox
    if (elem.addEventListener) {
        elem.addEventListener(eventName, handler, false);
    } 
    else if (elem.attachEvent) { // IE
        elem.attachEvent("on" + eventName, handler);
    }
}
if(jQuery) (function($){
    $.extend($.fn, {
        fileTree: function(o, h) {
            // Defaults
	    if( !o ) var o = {};
	    if( o.root == undefined ) o.root = '/';
	    if( o.script == undefined ) o.script = 'jqueryFileTree.php';
	    if( o.folderEvent == undefined ) o.folderEvent = 'click';
	    if( o.expandSpeed == undefined ) o.expandSpeed= 500;
	    if( o.collapseSpeed == undefined ) o.collapseSpeed= 500;
	    if( o.expandEasing == undefined ) o.expandEasing = null;
	    if( o.collapseEasing == undefined ) o.collapseEasing = null;
	    if( o.multiFolder == undefined ) o.multiFolder = true;
	    if( o.loadMessage == undefined ) o.loadMessage = 'Loading...';
	    
	    $(this).each( function() {
	        function showTree(c, t) {
		    $(c).addClass('wait');
		    $(".jqueryFileTree.start").remove();
                    var callback = function(data) {
			$(c).find('.start').html('');
			$(c).removeClass('wait').append(data);
			if( o.root == t ) {
			    $(c).find('UL:hidden').show(); 
			}
			else {
			    $(c).find('UL:hidden').slideDown({ duration: o.expandSpeed, easing: o.expandEasing });
			}
			bindTree(c);
                    };
                    if (o.getTree == undefined) {
			$.post(o.script, { dir: t }, callback);
                    } else {
			o.getTree(t, callback);
                    }
		}
			      
	        function bindTree(t) {
		    $(t).find('LI').each(function() {
                        bindEventHandler(this, o.folderEvent, 
					 function(eventObj) {
		            if( $(this).hasClass('directory') ) {
				if( $(this).hasClass('collapsed') ) {
				    // Expand
				    if( !o.multiFolder ) {
					$(this).parent().find('UL').slideUp({ duration: o.collapseSpeed, easing: o.collapseEasing });
					$(this).parent().find('LI.directory').removeClass('expanded').addClass('collapsed');
				    }
                                    $(this).find('UL').remove();
				    showTree( $(this), $(this).children("A").attr('rel').match( /.*\// ) );
				    $(this).removeClass('collapsed').addClass('expanded');
				} 
				else {
				    // Collapse
				    $(this).find('UL').slideUp({ duration: o.collapseSpeed, easing: o.collapseEasing });
				    $(this).removeClass('expanded').addClass('collapsed');
				}
			    }
		            knackerEvent(eventObj);
			    return false;
			});
                    });
                    $(t).find('LI A').each(function(eventObj) {
		        bindEventHandler(this, o.folderEvent, function() {
			    if (h != undefined) {
                                h(this);
                            }
			    knackerEvent(eventObj);
			    return false;
			});
                    });
		    // Prevent A from triggering the # on non-click events
		    if( o.folderEvent.toLowerCase != 'click' ) $(t).find('LI A').each(function() {
		        bindEventHandler(this, 'click', function(eventObj) { 
			    knackerEvent(eventObj); 
			    return false; 
			});
                    });
	        }
	        // Loading message
	        $(this).html('<ul class="jqueryFileTree start"><li class="wait">' + o.loadMessage + '<li></ul>');
    	        // Get the initial file list
	        showTree( $(this), escape(o.root) );
            });
        }
    });
})(jQuery);
