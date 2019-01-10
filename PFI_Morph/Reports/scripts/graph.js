let make_chart = function(
        source_name, 
        target_name, 
        canvas_id, 
        title, 
        forward_data, 
        backward_data,
        rolling_average_window=0) {
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

    // If a window size is given, make a second chart for the rolling average
    if (rolling_average_window > 0) {
        make_rolling_average_chart(...arguments);
    }
};

let make_rolling_average_chart = function(
        source_name, 
        target_name, 
        canvas_id, 
        title, 
        forward_data, 
        backward_data,
        rolling_average_window) {

    // Call make_chart() again with the averaged data
    make_chart(
        source_name, 
        target_name, 
        canvas_id + '-rolling-avg', 
        title + '[Rolling Avg]',
        rolling_average(forward_data, rolling_average_window),
        rolling_average(backward_data, rolling_average_window),
        0);
}

let rolling_average = function(data, window_size) {
    let results = [];
    let N = data.length - (window_size - 1);
    for (let i = 0; i < N; i++) {
        let sliding_window = data.slice(i, i + window_size)
        let avg = sliding_window.reduce((a, b) => a + b) / window_size;
        results.push(avg);
    }
    return results;
};
