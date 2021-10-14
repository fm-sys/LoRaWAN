package com.fmsys.logviewer;

import android.app.Dialog;
import android.graphics.Color;
import android.icu.text.SimpleDateFormat;
import android.os.Bundle;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.TooltipCompat;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.github.mikephil.charting.animation.Easing;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.Legend;
import com.github.mikephil.charting.components.LimitLine;
import com.github.mikephil.charting.components.MarkerView;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.ValueFormatter;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;
import com.google.android.material.datepicker.MaterialDatePicker;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;
import java.util.TimeZone;

public class MainActivity extends AppCompatActivity {

    private static final SimpleDateFormat formatter_HHmm = new SimpleDateFormat("HH:mm", Locale.GERMANY);
    private static final SimpleDateFormat formatter_ddMMyy = new SimpleDateFormat("dd.MM.yy", Locale.GERMANY);
    private static final SimpleDateFormat formatter_ddMMyyyy = new SimpleDateFormat("dd.MM.yyyy", Locale.GERMANY);

    LineChart chart;
    SwipeRefreshLayout refreshLayout;
    Button button;
    String currentChart = "d1_Temperatur";
    Long timestamp = new Date().getTime() / 1000;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setTitle(R.string.app_name);

        refreshLayout = findViewById(R.id.swipe_refresh);
        refreshLayout.setOnRefreshListener(this::setData);

        button = findViewById(R.id.set_date);
        button.setText(formatter_ddMMyyyy.format(new Date()));
        button.setOnClickListener(v -> changeDateDialog());

        chart = findViewById(R.id.chart);

        // background color
        chart.setBackgroundColor(Color.WHITE);

        // disable description text
        chart.getDescription().setEnabled(false);

        // enable touch gestures
        chart.setDrawGridBackground(true);

        // enable scaling and dragging
        chart.setDragEnabled(true);
        chart.setScaleEnabled(true);

        // create marker to display box when values are selected
        MarkerView mv = new ChartMarkerView(this, R.layout.marker_view);
        mv.setChartView(chart);
        chart.setMarker(mv);


        XAxis xAxis = chart.getXAxis();
        YAxis yAxis = chart.getAxisLeft();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setGranularity(1); // smallest allowed step

        xAxis.setValueFormatter(new ValueFormatter() {
            @Override
            public String getFormattedValue(float value) {
                return formatter_HHmm.format(new Date((long) value * 1000));
            }
        });


        // disable dual axis (only use LEFT axis)
        chart.getAxisRight().setEnabled(false);


        // // Create Limit Lines // //

//        LimitLine ll1 = new LimitLine(150f, "Upper Limit");
//        ll1.setLineWidth(4f);
//        ll1.enableDashedLine(10f, 10f, 0f);
//        ll1.setLabelPosition(LimitLine.LimitLabelPosition.RIGHT_TOP);
//
//        LimitLine ll2 = new LimitLine(-30f, "Lower Limit");
//        ll2.setLineWidth(4f);
//        ll2.enableDashedLine(10f, 10f, 0f);
//        ll2.setLabelPosition(LimitLine.LimitLabelPosition.RIGHT_BOTTOM);
//
//        // draw limit lines behind data instead of on top
//        yAxis.setDrawLimitLinesBehindData(true);
//        xAxis.setDrawLimitLinesBehindData(true);
//
//        // add limit lines
//        yAxis.addLimitLine(ll1);
//        yAxis.addLimitLine(ll2);

        // add data
        setData();

        // legend (only possible after setting data)
        Legend l = chart.getLegend();
        l.setForm(Legend.LegendForm.LINE);

    }

    private void changeDateDialog() {
        final MaterialDatePicker<Long> dateDialog = MaterialDatePicker.Builder
                .datePicker()
                .setSelection(timestamp * 1000)
                .build();
        dateDialog.addOnPositiveButtonClickListener(newTimestampMillis -> {
            timestamp = newTimestampMillis / 1000;
            button.setText(formatter_ddMMyyyy.format(new Date(newTimestampMillis)));
            // trigger refresh
            setData();
        });
        dateDialog.show(getSupportFragmentManager(), "date_dialog");
    }

    private void setData() {


        RequestQueue queue = Volley.newRequestQueue(this);
        String url = "https://iotplotter.com/api/v2/feed/829369784150976580?epoch=" + timestamp;

        StringRequest stringRequest = new StringRequest(Request.Method.GET, url,
                response -> {
                    ArrayList<Entry> values = new ArrayList<>();
                    Set<String> graphs = new HashSet<>();

                    try {
                        JSONObject responseObject = new JSONObject(response);
                        JSONArray js = responseObject.names();
                        assert js != null;
                        for (int i = 0; i < js.length(); i++) {
                            JSONObject dataObject = responseObject.getJSONObject(js.getString(i)).getJSONObject("data");

                            JSONArray graphNames = dataObject.names();
                            assert graphNames != null;
                            for (int j = 0; j < graphNames.length(); j++) {
                                graphs.add(graphNames.getString(j));

                                if (graphNames.getString(j).equals(currentChart)) {
                                    values.add(new Entry(js.getInt(i), (float) dataObject.getDouble(currentChart)));
                                }
                            }
                        }
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }

                    LineDataSet set1 = (LineDataSet) chart.getData().getDataSetByIndex(0);

                    Log.d("DATA", values.toString());
                    Log.d("GRAPHS", graphs.toString());

                    String[] array = graphs.toArray(new String[0]);
                    Arrays.sort(array);

                    final ArrayAdapter<String> spinnerAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, array);
                    ((Spinner) findViewById(R.id.chart_spinner)).setAdapter(spinnerAdapter);

                    if (currentChart != null && graphs.contains(currentChart)) {
                        final int spinnerPosition = Arrays.asList(array).indexOf(currentChart);
                        ((Spinner) findViewById(R.id.chart_spinner)).setSelection(spinnerPosition);
                    } else if (array.length > 0) {
                        currentChart = array[0];
                        setData();
                    }

                    ((Spinner) findViewById(R.id.chart_spinner)).setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                        @Override
                        public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
                            if (!array[position].equals(currentChart)) {
                                currentChart = array[position];
                                setData();
                            }
                        }

                        @Override
                        public void onNothingSelected(AdapterView<?> parentView) {
                            // nothing
                        }

                    });

                    set1.setValues(values);
                    set1.notifyDataSetChanged();
                    chart.getData().notifyDataChanged();
                    chart.notifyDataSetChanged();

                    // draw points over time
                    chart.animateY(500, Easing.EaseOutCubic);

                    if (values.size() > 0) {
                        Entry latestEntry = values.get(values.size() - 1);
                        Date lastUpdate = new Date((long) (latestEntry.getX() * 1000));


                        ((TextView) findViewById(R.id.last_update)).setText(DateUtils.isToday(lastUpdate.getTime()) ?
                                formatter_HHmm.format(lastUpdate) :
                                formatter_ddMMyy.format(lastUpdate));
                        TooltipCompat.setTooltipText(findViewById(R.id.last_update), lastUpdate.toString());

                        ((TextView) findViewById(R.id.current_value)).setText(latestEntry.getY() + (currentChart.endsWith("CO2") ? " ppm" : " °C"));
                    }

                    refreshLayout.setRefreshing(false);


                }, error -> {
            LineDataSet set1 = (LineDataSet) chart.getData().getDataSetByIndex(0);
            set1.setValues(new ArrayList<>());
            set1.notifyDataSetChanged();
            chart.getData().notifyDataChanged();
            chart.notifyDataSetChanged();
            chart.animateY(0, Easing.EaseOutCubic);

            ((TextView) findViewById(R.id.last_update)).setText("");
            ((TextView) findViewById(R.id.current_value)).setText("");

            Toast.makeText(MainActivity.this, R.string.no_data, Toast.LENGTH_LONG).show();
        });

        queue.add(stringRequest);


        LineDataSet set1;

        if (chart.getData() == null || chart.getData().getDataSetCount() <= 0) {
            // create a dataset and give it a type
            set1 = new LineDataSet(new ArrayList<>(), "Temperatur (in °C)");

            // black lines and points
            set1.setColor(Color.BLACK);

            // line thickness
            set1.setLineWidth(4f);
            set1.setDrawCircles(false);
            //set1.setMode(LineDataSet.Mode.HORIZONTAL_BEZIER);

            // customize legend entry
            set1.setFormLineWidth(2f);
            set1.setFormSize(15.f);

            // text size of values: 0 -> no text
            set1.setValueTextSize(0f);

            ArrayList<ILineDataSet> dataSets = new ArrayList<>();
            dataSets.add(set1); // add the data sets

            // create a data object with the data sets
            LineData data = new LineData(dataSets);

            // set data
            chart.setData(data);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.info) {
            showImage();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public void showImage() {
        Dialog builder = new Dialog(this);
        builder.setContentView(getLayoutInflater().inflate(R.layout.info_dialog, null));
        builder.show();
    }
}
