/// <reference path="../../jquery.cookie.js" />
// Please note that NONE OF THIS IS NECESSARY for the lines to show up,
// rather it's just so they're collapsible and will remember their state
// between page loads.
(function ($, window, undefined) {

    var _, win, elements;

    win = $(window);

    elements = $(".flist");

    _ = {
        verbose: false,

        getOpenIDsForList: function (list) {
            /// <summary>Returns the string array of open id's from a cookie.</summary>
            var openIDs = list.data("openIDs");
            var listID = list.attr("id");

            if (!openIDs) {
                openIDs = $.cookie("flist_" + listID);

                if (openIDs != null) {
                    try {
                        openIDs = JSON.parse(openIDs) || [];
                    } catch (e) {
                        if (_.verbose === true) {
                            console.warn("flist enhancment: openIDs parse failed. Error: %o", e);
                        }
                        openIDs = [];
                    }
                } else {
                    openIDs = [];
                }
            }
            return openIDs;
        },

        setOpenIDsForList: function (list, ids) {
            /// <summary>Saves the string array of open id's to a cookie.</summary>
            var listID = list.attr("id");
            list.data("openIDs", ids);
            $.cookie("flist_" + listID, JSON.stringify(ids), {
                path: "/"
            });
            // leaving expires not set for a 'session' cookie
        },

        addIcons: function (list) {
            /// <summary>Add the arrows to each item that contains a list</summary>
            var childLists = list.find("ul");
            $.each(childLists, function (index, childList) {
                var parentLi, icon, iconType, spn;

                childList = $(childList);
                parentLi = childList.closest("li");
                iconType = childList.is(":visible") ? '&#x25be;' : '&#x25b8;';
                spn = $("<span>").html(iconType);
                icon = $('<a class="list-icon"></a>').append(spn);

                parentLi.prepend(icon);
                icon.click(_.iconClicked);
            });
        },

        openAllOpenIDs: function (list) {
            /// <summary>
            /// Expands all list items from the cookie.
            /// Do not need this if the html helper is used to determine visibility
            /// at page load which reduces page jumpiness for older browsers.
            /// </summary>
            if (!list.attr("id")) {
                return;
            }

            openIDs = _.getOpenIDsForList(list);
            $.each(openIDs, function (index, id) {
                var item = $("#" + id);
                if (item.length > 0) {
                    _.toggleItem($("#" + id), true /*expand*/ );
                }
            });
        },

        toggleItem: function (li, expand) {
            /// <summary>
            /// Toggles the visibility of the child list.
            /// If expand is set to true or false, it will expand or collapse accordingly.
            /// </summary>
            var list = li.closest(".flist");
            var openIDs = _.getOpenIDsForList(list);
            var isVisible, spliceIndex;
            var hide = function () {
                childList.hide();
                li.find(".list-icon span").first().html("&#x25b8;");
                spliceIndex = $.inArray(li.attr("id"), openIDs);
                openIDs.splice(spliceIndex, 1);
                _.setOpenIDsForList(list, openIDs);
            };
            var show = function () {
                var id = li.attr("id");
                // jch** - if id is undefined the list will not remember the toggle position
                // should add a verbose flag and add a console message.
                if (_.verbose === true) {
                    console.warn("open id = %o", id);
                }
                childList.show();
                li.find(".list-icon span").first().html("&#x25be;"); // down arrow
                if ($.inArray(id, openIDs) === -1) {
                    openIDs.push(id);
                    _.setOpenIDsForList(list, openIDs);
                }
            };

            childList = li.children("ul");
            isVisible = childList.is(":visible");

            if (expand === true) {
                show();
            } else if (expand === false) {
                hide();
            } else if (isVisible) {
                hide();
            } else {
                show();
            }
        },

        iconClicked: function (event) {
            var parentLi;
            var childList;
            var icon = $(this);

            //event.preventDefault();
            parentLi = icon.closest("li");
            _.toggleItem(parentLi);
        },

        openToSelectedItem: function (list) {
            /// <summary>Finds items with the .selected class name and opens the parent nodes.</summary>
            var selected = list.find(".selected");
            var link = selected.find("a").not(".list-icon").first();

            link.css("outline", "0");
            setTimeout(function () {
                var focused = $(document.activeElement);
                if (focused.is(":input") === false) {
                    link.focus();
                }
            }, 0);

            $.each(selected, function (i, item) {
                item = $(item);
                var parent = item.parents("li");

                _.toggleItem(item, true);

                // open all parents
                while (parent.length > 0) {
                    _.toggleItem(parent, true);
                    parent = parent.parents("li");
                }
            });
        },

        setAutoExpand: function (list) {
            var nodes = list.find("a.auto-expand");

            $.each(nodes, function (index, node) {
                node = $(node);
                node.click(_.iconClicked);
            });
        }
    };

    elements.each(function (index, list) {

        list = $(list);

        _.addIcons(list);

        _.openAllOpenIDs(list);

        _.openToSelectedItem(list);

        _.setAutoExpand(list);

        // open the root node if one
        _.toggleItem(list.find("li:first"), true);
    });

})(jQuery, window);
