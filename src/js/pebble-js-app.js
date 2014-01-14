Pebble.addEventListener("ready", function() {
	Pebble.sendAppMessage({"0": "ready"});
});

Pebble.addEventListener("appmessage", function(e) {
	var method = e.payload["0"];
	if (method == "story"){
	  var req = new XMLHttpRequest();
	  req.open('GET', 'http://feeds.bbci.co.uk/news/rss.xml?edition=uk', true);
	  req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
		  if(req.status == 200) {
		    var parser = new DOMParser();
            var xmlDoc = parser.parseFromString(req.responseText, "text/xml");
			var items = xmlDoc.getElementsByTagName("item");
			var title;
			if (items.length == 0)
				title = "No News :-(";
		    else {
				var titleNode = items[0].getElementsByTagName("title");
				if (titleNode.length == 0)
					title = "Item has no title :-(";
				else
					title = titleNode[0].textContent;
			}
			Pebble.sendAppMessage({"0": method, "1": title});
		  } else {
			console.log("Error");
		  }
		}
	  }
	  req.send(null);
	} else {
		console.log("Unknown appmessage method [" + method + "]");
	}
});