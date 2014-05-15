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
		    var s = req.responseText;
			var i = s.indexOf("<item>");
			if (i >= 0){
			  s = s.substr(i);
			  i = s.indexOf("<title>");
			  if (i >= 0){
			    s = s.substr(i+7);
				i = s.indexOf("<");
			    if (i >= 0){
			      s = s.substr(0, i);
			      Pebble.sendAppMessage({"0": method, "1": s});
				}
			  }
			}
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