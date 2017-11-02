$(function() {
	const ESP32_IP   = "192.168.1.99";
	const ESP32_PORT = 80;
	var fileNameDeferred = null;
	const FILE_DIRECTORY = 2; // Type of file that is a directory
	const FILE_REGULAR   = 1; // Type of file that is a regular file
	var bleClient = null;
	
	function getFileNameDialog() {
		fileNameDeferred = jQuery.Deferred();
		$("#fileNameText").val("");
		$("#fileNameDialog").dialog("open");
		return fileNameDeferred;
	}
	
	
	function doRest(path, callback, obj) {
		obj.url      = "http://" + ESP32_IP + ":" + ESP32_PORT + path;
		obj.dataType = "json";
		obj.success  = function(data, textStatus, jqXHR) {
			if (callback) {
				callback(data);
			}
		};
		obj.error    = function(jqXHR, textStatus, errorThrown) {
			console.log("AJAX error");
			debugger;
		};
		jQuery.ajax(obj);
	} // doRest
	
	function getData(path, callback) {
		doRest(path, callback, {
			method   : "GET"
		});
	} // getData
	
	function deleteData(path, callback, _data) {
		doRest(path, callback, {
			method   : "DELETE",
			data: _data
		});
	} // deleteData

	function postData(path, callback, _data) {
		doRest(path, callback, {
			method   : "POST",
			data: _data
		});
	} // postData
	
	
	function addFileDirectory(childrenArray, fileEntries) {
/*
 * File entries is an array of objects
 * path: Full path name
 * name: name of file/directory
 * directory: true/false (is this a directory)
 * dir: [child path entries]
 */
		for (var i=0; i<fileEntries.length; i++) {
			var newChild = {
				text: fileEntries[i].name,
				icon: fileEntries[i].directory?"jstree-folder":"jstree-file",
				data: {
					path: fileEntries[i].path,
					directory: fileEntries[i].directory
				}
			}
			if (fileEntries[i].directory) {
				var newChildren = [];
				addFileDirectory(newChildren, fileEntries[i].dir);
				newChild.children = newChildren;
			}
			childrenArray.push(newChild);
		}
	} // addFileDirectory
	
	
	function buildFileSystemTree(path) {  // TODO refactor to match new regex
		if (path.length == 0) {
			return;
		}
		console.log("Get the directory at: " + path);
		getData("/ESP32/FILE" + path, function(data) {
			// We now have data!
			var root = {
				text: path,
				state: {
					opened: true
				},
				icon: "jstree-folder",
				children: [],
				data: {
					path: path,
					directory: true
				}
			}
			addFileDirectory(root.children, data.dir);
			$('#fileTree').jstree(true).settings.core.data = root;
			$("#fileTree").jstree(true).refresh();
		});
	} // buildFileSystemTree

	
	function showGPIODialog(gpio) {
		$("#gpioDialog").dialog("option", {
			title: "GPIO " + gpio
		});
		getData("/ESP32/GPIO", function(data) {
			$("#gpioJsonText").val(JSON.stringify(data, null, "  "));

			$("#gpioDialog_OUT_REG").text(data.out[gpio]);
			$("#gpioDialog_IN_REG").text(data.in[gpio]);
			$("#gpioDialog_ENABLE_REG").text(data.enable[gpio]);
			$("#gpioDialog_STATUS_REG").text(data.enable[gpio]);
			if (data.enable[gpio]) {
				$("#inputGpioDialog").prop("checked", false).button("refresh");
				$("#outputGpioDialog").prop("checked", true).button("refresh");
			} else {
				$("#inputGpioDialog").prop("checked", true).button("refresh");
				$("#outputGpioDialog").prop("checked", false).button("refresh");
			}
			$("#inputGpioDialog").attr("data-gpio", gpio);
			$("#outputGpioDialog").attr("data-gpio", gpio);
			$("#gpioDialog").dialog("open");
		})
	} // showGPIODialog

	
	function typeToString(type) {
		switch (type) {
		case 0x00:
			return "App";
		case 0x01:
			return "Data";
		default:
			return type.toString();
		}
	} // typeToString

	
	function subTypeToString(subtype) {
		switch (subtype) {
		case 0x00:
			return "Factory";
		case 0x01:
			return "Phy";
		case 0x02:
			return "NVS";
		case 0x03:
			return "Coredump";
		case 0x80:
			return "ESP HTTPD";
		case 0x81:
			return "FAT";
		case 0x82:
			return "SPIFFS";
		default:
			return subtype.toString();
		}
	} // subTypeToString

	
	console.log("ESP32Explorer loading");
	$("#myTabs").tabs();
	$("#systemTabs").tabs();
	$("#gpioTabs").tabs();
	$("#wifiTabs").tabs();
	$("#i2cTabs").tabs();
	$("#bleTabs").tabs();
	
	$("#systemLoggingTab [name='systemLogging']").checkboxradio().on("change", function(event) {
		postData("/ESP32/LOG/SET", null, 
		{
			"level":$(event.target).attr("data-logLevel")
		});
	});
	
	$("#inputGpioDialog").checkboxradio().on("change", function(event) {
		var gpio = $(event.target).attr("data-gpio");
		postData("/ESP32/GPIO/DIRECTION/INPUT", null, {"gpio": gpio});
		//debugger;
	});
	$("#outputGpioDialog").checkboxradio().on("change", function(event) {
		var gpio = $(event.target).attr("data-gpio");
		postData("/ESP32/GPIO/DIRECTION/OUTPUT", null, {"gpio": gpio});
		//debugger;
	});
	
	// Draw the GPIO table.
	var table = $("#gpioTable");
	for (var j=0; j<5; j++) {
		var tr = $("<tr>");
		for (var i=0; i<8; i++) {
			var gpioNum = j*8 + i;
			var td = $("<td>");
			var div;
			var wDiv;
			wDiv = $("<div class='flexHorizCenter'>");
			div = $("<div>");
			div.attr("id", "gpio" + gpioNum + "Icon");
			wDiv.append(div);
			
			div = $("<div class='flexHorizCenter' style='margin-left: 4px; font-size: x-large;'>");
			div.text("" + gpioNum);
			wDiv.append(div);
			
			td.append(wDiv);
			
			wDiv = $("<div class='flexHorizCenter'>");
			var checkBox = $("<input type='checkbox'>");
			checkBox.attr("data-gpio", gpioNum );
			checkBox.click(function(event){
				var gpio = event.target.dataset.gpio;
				var state = event.target.checked;
				console.log("State of " + gpio + " now " + state);
				if (state) {
					postData("/ESP32/GPIO/SET", null,
					{"gpio": gpio});
				} else {
					postData("/ESP32/GPIO/CLEAR", null,
					{"gpio": gpio});
				}
			});
			checkBox.attr("id", "gpio" + gpioNum + "Checkbox");
			wDiv.append(checkBox);
			
			div = $("<div class='flexHorizCenter'>");
			var info = $("<div class='infoImage'>");
			info.attr("data-gpio", gpioNum );
			$(info).on("click", function(event){
				var selectedGpio = event.target.dataset.gpio;
				console.log("Clicked!: " + selectedGpio);
				showGPIODialog(selectedGpio);
				//debugger;
			});
			div.append(info);
			wDiv.append(div);
			td.append(wDiv);

			tr.append(td);
		}
		table.append(tr);
	} // End of loop for GPIO items.

	
	$("#systemRefreshButton").button().click(
		function() {
			getData("/ESP32/SYSTEM", function(data) {

				$("#systemJsonText").val(JSON.stringify(data, null, "  ")); // Log the JSON text
				$("#espVersionSystemGeneralTab").text(data.espVersion);
				$("#coresSystemGeneralTab").text(data.cores);
				$("#revisionSystemGeneralTab").text(data.revision);
				$("#freeHeapSystemGeneralTab").text(data.freeHeap);
				$("#timeSystemGeneralTab").text(data.time);	
				$("#taskCountFreeRTOSTab").text(data.taskCount);
				
				// Load the partition table information.
				var table = $("#partitionTable");
				table.find("> tr").remove();
				// debugger;
				data.partitions.sort(function(a, b) {
					return a.address - b.address
				});
				for (var i = 0; i < data.partitions.length; i++) {
					var row = $("<tr>");
					row.append("<td>" + typeToString(data.partitions[i].type) + "</td>");
					row.append("<td>" + subTypeToString(data.partitions[i].subType) + "</td>");
					row.append("<td>" + data.partitions[i].size + "</td>");
					row.append("<td>0x" + data.partitions[i].address.toString(16) + "</td>");
					row.append("<td>"	+ (data.partitions[i].encrypted ? "Yes" : "No")	+ "</td>");
					row.append("<td>" + data.partitions[i].label + "</td>");
					table.append(row);
				}
				// End load the partition table information
				
				// Load the task status information
				var table = $("#taskStatusTable");
				table.find("> tr").remove();
				// debugger;
				if (data.taskStatus != null) {
					for (var i = 0; i < data.taskStatus.length; i++) {
						var row = $("<tr>");
						row.append("<td>" + data.taskStatus[i].name + "</td>");
						row.append("<td>" + data.taskStatus[i].taskNumber + "</td>");
						row.append("<td>" + data.taskStatus[i].priority + "</td>");
						row.append("<td>" + data.taskStatus[i].stackHighWater + "</td>");
						row.append("<td>" + data.taskStatus[i].state + "</td>");
						table.append(row);
					}   // End process each task in the task status data.
				}      // Status data for tasks was present.
				// End load the task status information
			}); // getData
		}
	);

	$("#wifiRefreshButton").button().click(function() {  
		getData("/ESP32/WIFI", function(data) {
			$("#wifiJsonText").val(JSON.stringify(data, null, "  "));
			$("#modeWifi").text(data.mode);
			$("#staMacWifi").text(data.staMac);
			$("#apMacWifi").text(data.apMac);
			$("#staSSIDWifi").text(data.staSSID);
			$("#apSSIDWifi").text(data.apSSID);
			$("#apIpWifi").text(data.apIpInfo.ip);
			$("#apGwWifi").text(data.apIpInfo.gw);
			$("#apNetmaskWifi").text(data.apIpInfo.netmask);
			$("#staIpWifi").text(data.staIpInfo.ip);
			$("#staGwWifi").text(data.staIpInfo.gw);
			$("#staNetmaskWifi").text(data.staIpInfo.netmask);			
		})
	});

	$("#bleScanButton").button().click(function(){
		getData("/ESP32/BLE/CLIENT/SCAN", function(data) {
			$("#bleJsonText").val(JSON.stringify(data, null, "  "));
			$('#bleTree').jstree(true).settings.core.data = data;
			$('#bleTree').jstree(true).refresh();
		})
	});

	$("#bleConnectButton").button().click(function(){
		var sel = bleClient;
		postData("/ESP32/BLE/CLIENT/CONNECT", function(data) {
			$("#bleJsonText").val(JSON.stringify(data, null, "  "));
			$('#bleServicesTree').jstree(true).settings.core.data = data;
			$('#bleServicesTree').jstree(true).refresh();
		}, 
		{
			"connect" : bleClient.node.id
		})
	});

	$("#gpioRefreshButton").button().click(function() {
		getData("/ESP32/GPIO", function(data) {
			$("#gpioJsonText").val(JSON.stringify(data, null, "  "));
			// Loop through each of the GPIO items.
			for (var i=0; i<40; i++) {
				$("#gpio" + i + "Checkbox" ).attr("disabled", !data.enable[i]);
				if (data.enable[i] == true) {
					$("#gpio" + i + "Checkbox" ).attr("checked", data.out[i]);
				} else {
					$("#gpio" + i + "Checkbox" ).attr("checked", data.in[i]);
				}
				$("#gpio" + i + "Icon").toggleClass("inputImage", !data.enable[i]);
				$("#gpio" + i + "Icon").toggleClass("outputImage", data.enable[i]);
			}
		});
	});

	$("#i2sRefreshButton").button().click(function() {
		getData("/ESP32/I2S", function(data) {
			$("#i2sJsonText").val(JSON.stringify(data, null, "  "));
		})
	});
	
	$("#fileSystemRefreshButton").button().click(function() {
		var path = $("#rootPathText").val().trim();
		if (path.length == 0) {
			return;
		}
		buildFileSystemTree(path);
	});
	
	$('#fileTree').jstree({
		plugins: ["contextmenu"],
		contextmenu: {
			items: function(node) {
				var n = $('#fileTree').jstree(true).get_node(node);
				var items = {};
				if (n.data.directory) {
					items.createDir = {
			   		label: "Create",
			   		action: function(x) {
			   			var n= $('#fileTree').jstree(true).get_node(x.reference);
			   			console.log("Create a directory as a child of " + n.data.path);
			   			getFileNameDialog().done(function(fileName) {
				   			postData("/ESP32/FILE" + n.data.path + "/" + fileName + "?directory=true");
			   			});
			   			//debugger;
			   		}
			   	}
				}
				else {
					items.downloadFile = {
			   		label: "Download",
			   		action: function(x) {
			   			var n = $('#fileTree').jstree(true).get_node(x.reference);
			   			console.log("Download the file called " + n.data.path);
			   			// Form a web socket to download the data ...
			   			var ws = new WebSocket("ws://" + ESP32_IP + ":" + ESP32_PORT);
			   			ws.onopen = function() {
			   				console.log("Web Socket now open!  Sending command ...");
			   				ws.send(JSON.stringify({
			   					command: "getfile",
			   					path: n.data.path
			   				}));
			   				ws.close();
			   			} 
			   		}
					};
					items.deleteFile = {
			   		label: "Delete",
			   		action: function(x) {
			   			var n= $('#fileTree').jstree(true).get_node(x.reference);
			   			console.log("Delete the file called " + n.data.path);
			   			deleteData("/ESP32/FILE" + n.data.path);
			   		}
			   	};
				}
			   return items;
			}
		}
	});
	
	$("#fileUploadForm").fileupload({
		add: function(e, data) {

			var selected = $("#fileTree").jstree("get_selected");

			if (selected.length > 0) {
				var node = $("#fileTree").jstree(true).get_node(selected[0]);
				data.formData = { path: node.data.path };
				data.submit();
			}
			//debugger;
		},
		done: function(e, data) {
			var path = $("#rootPathText").val().trim();
			if (path.length == 0) {
				return;
			}
			buildFileSystemTree(path);	
		}
	});
	
	$("#gpioDialog").dialog({
		autoOpen: false,
		modal: true,
		buttons: [
			{
				text: "Close",
				click: function() {
					$("#gpioDialog").dialog("close");
				}
			}
		]
	});
	
	$("#fileNameDialog").dialog({
		autoOpen: false,
		modal: true,
		title: "File name",
		width: 305,
		buttons: [
			{
				text: "OK",
				click: function() {
					fileNameDeferred.resolve($("#fileNameText").val());
					$("#fileNameDialog").dialog("close");
				}
			}
		]
	});
	
	$("#i2cScanButton").button().click(function() {  //TODO add to scan which one port?
		getData("/ESP32/I2C/SCAN", function(data) {
			$("#i2cJsonText").val(JSON.stringify(data, null, "  "));
			// Loop through each of the I2C address.
			for (var i=3; i<123; i++) {
				if (data.present[i-3]) {
					$("#i2c" + i + "Icon").addClass("checkImage");
					$("#i2c" + i + "Icon").removeClass("crossImage");
				} else {
					$("#i2c" + i + "Icon").removeClass("checkImage");
					$("#i2c" + i + "Icon").addClass("crossImage");
				}
			}
		});
	});

	$("#i2cReadButton").button().click(function() { //TODO populate read values
		var address = $("#i2c_adr").val();
		var reg     = $("#i2c_reg").val();
		var count   = $("#i2c_cnt").val();
		var data    = $("#i2c_data").val();
		var sda     = $("#i2c_sda").val();
		var scl     = $("#i2c_scl").val();
		postData("/ESP32/I2C/COMMAND/READ", function(data) {
			$("#i2cJsonText").val(JSON.stringify(data, null, "  "))}, 
		{
			"address": address,  //i2c address to read
			"register": reg,  //register to read
			"bytesCount": count, // bytes to read
			"data": data, // do we need this?
			"sda": sda,   // do we need this?
			"scl": scl    // do we need this?
		});
	});

	$("#i2cWriteButton").button().click(function() { 
		var address = $("#i2c_adr").val();
		var reg     = $("#i2c_reg").val();
		var count   = $("#i2c_cnt").val();
		var data    = $("#i2c_data").val();
		var sda     = $("#i2c_sda").val();
		var scl     = $("#i2c_scl").val();
		postData("/ESP32/I2C/COMMAND/WRITE", function(data) {
			$("#i2cJsonText").val(JSON.stringify(data, null, "  "));
		},
		{
			"address": address,
			"register": reg,
			"bytesCount": count,
			"data": data,
			"sda": sda, // do we need this?
			"scl": scl  // do we need this?
		});
	});

	$("#i2cCloseButton").button().click(function() {
		var port = $('input[name=i2c_port]:checked').val();
		postData("/ESP32/I2C/DEINIT", null, 
		{
			"port": port // I2C_NUM_0(1)
		});
	});

	$("#i2cInitButton").button().click(function() {
		var port  = $('input[name=i2c_port]:checked').val();
		var sda   = $("#i2c_sda").val();
		var scl   = $("#i2c_scl").val();
		var speed = $("#i2c_speed").val();
		var mode  = $('input[name=i2c_mode]:checked').val();
		postData("/ESP32/I2C/INIT", null, 
		{
			"port": port,  //i2c port to init --> I2C_NUM_0(1)
			"sda": sda,    //SDA PIN
			"scl": scl,    //SCL PIN
			"speed": speed,//SPEED
			"mode": mode   //MODE --> SLAVE/MASTER
		});
	});

	
	// Build the I2C scan table
	var table = $("#i2cScanTable");
	for (var j=0; j<8; j++) { // For each row.
		var tr = $("<tr>");
		for (var i=0; i<16; i++) { // For each column
			var i2cNum = j*16 + i;
			if (i2cNum < 3) {   // The cells for addresses 0, 1 and 2 should be empty.
				tr.append($("<td>"));
			} else {
				if(i2cNum < 120){
					var td = $("<td>");
					var div;
					var wDiv;
					wDiv = $("<div class='flexHorizCenter'>");
					div = $("<div>");
					div.attr("id", "i2c" + i2cNum + "Icon");
					wDiv.append(div);
					
					div = $("<div class='flexHorizCenter' style='margin-left: 4px; font-size: x-large;'>");
					div.text("" + i2cNum);
					wDiv.append(div);
					
					td.append(wDiv);
					tr.append(td);
				}
			} // Cell > 2.
		} // End for each column.
		table.append(tr);
	} // End for each row.

	$('#bleTree').on('select_node.jstree', function(node, selected, event){
		bleClient = selected;
	});
		
	$('#bleTree').jstree({
	  "core" : {
	  	"multiple":false,
	    "animation" : 0,
	    "check_callback" : true,
	    "themes" : { "stripes" : true }
	  },
	  "types" : {
	    "#" : {
	      "max_children" : 10,
	      "max_depth" : 4,
	      "valid_children" : ["root", "device"]
	    },
	    "root" : {
	      "icon" : "/static/3.3.4/assets/images/tree_icon.png",
	      "valid_children" : ["device"]
	    },
	    "device" : {
	      "valid_children" : ["service"]
	    },
	    "service" : {
	      "valid_children" : ["characteristic"]
	    },
	    "characteristic" : {
	      "valid_children" : []
	    }
	  },
	  "plugins" : []
	});

	$('#bleServicesTree').jstree({
	  "core" : {
	  	"multiple":false,
	    "animation" : 0,
	    "check_callback" : true,
	    "themes" : { "stripes" : true }
	  },
	  "types" : {
	    "#" : {
	      "max_children" : 10,
	      "max_depth" : 4,
	      "valid_children" : ["root"]
	    },
	    "root" : {
	      "icon" : "/static/3.3.4/assets/images/tree_icon.png",
	      "valid_children" : ["default", "service"]
	    },
	    "default" : {
	      "valid_children" : ["default", "service"]
	    },
	    "service" : {
	      "icon" : "/static/3.3.4/assets/images/tree_icon.png",
	      "valid_children" : ["characteristic"]
	    },
	    "characteristic" : {
	      "icon" : "/static/3.3.4/assets/images/tree_icon.png",
	      "valid_children" : ["default"]
	    }
	  },
	  "plugins" : []
	});

});