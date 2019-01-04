let make_chart = function(
        source_name, 
        target_name, 
        canvas_id, 
        title, 
        forward_data, 
        backward_data) {
	let ctx = document.getElementById(canvas_id).getContext('2d');
    
    let longest_length = Math.max(forward_data.length, backward_data.length);
    let labels = Array(longest_length).fill(0).map((_, i) => i);

	let chart = new Chart(ctx, {
		type: 'line',
		data: {
            labels: labels,
			datasets: [{
				label: `${source_name} -> ${target_name}`,	
                backgroundColor: 'rgb(25, 126, 126, 0.5)',
                borderColor: 'rgb(0, 78, 78)',
                data: forward_data,
                lineTension: 0,
            }, {
				label: `${target_name} -> ${source_name}`,	
                backgroundColor: 'rgb(210, 118, 41, 0.5)',
                borderColor: 'rgb(130, 59, 0)',
                data: backward_data,
                lineTension: 0,
			}],
		},
		options: {
			title: {
                display: true,
                text: title
            },
            responsive:false
		},
	});
};

/*
var ctx = document.getElementById('myChart').getContext('2d');
var chart = new Chart(ctx, {
    // The type of chart we want to create
    type: 'line',

    // The data for our dataset
    data: {
        labels: ["January", "February", "March", "April", "May", "June", "July"],
        datasets: [{
            label: "My First dataset",
            backgroundColor: 'rgb(255, 99, 132)',
            borderColor: 'rgb(255, 99, 132)',
            data: [0, 10, 5, 2, 20, 30, 45],
        }]
    },

    // Configuration options go here
    options: {}
});
*/
