package com.fmsys.logviewer;

import android.app.Dialog;
import android.graphics.Color;
import android.icu.text.SimpleDateFormat;
import android.os.Bundle;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.github.mikephil.charting.animation.Easing;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.Legend;
import com.github.mikephil.charting.components.LimitLine;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

public class MainActivity extends AppCompatActivity {

    LineChart chart;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setTitle(R.string.app_name);


        chart = findViewById(R.id.chart);

        // background color
        chart.setBackgroundColor(Color.WHITE);

        // disable description text
        chart.getDescription().setEnabled(false);

        // enable touch gestures
        chart.setTouchEnabled(true);
        chart.setDrawGridBackground(true);

//        // create marker to display box when values are selected
//        MyMarkerView mv = new MyMarkerView(this, R.layout.custom_marker_view);
//
//        // Set the marker to the chart
//        mv.setChartView(chart);
//        chart.setMarker(mv);

        // enable scaling and dragging
        chart.setDragEnabled(true);
        chart.setScaleEnabled(true);


        XAxis xAxis = chart.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);

            // vertical grid lines
            //xAxis.enableGridDashedLine(10f, 10f, 0f);


        YAxis yAxis = chart.getAxisLeft();

            // disable dual axis (only use LEFT axis)
            chart.getAxisRight().setEnabled(false);

            // horizontal grid lines
            //yAxis.enableGridDashedLine(10f, 10f, 0f);

            // axis range
            //yAxis.setAxisMaximum(200f);
            //yAxis.setAxisMinimum(-50f);


        // // Create Limit Lines // //

        LimitLine ll1 = new LimitLine(150f, "Upper Limit");
        ll1.setLineWidth(4f);
        ll1.enableDashedLine(10f, 10f, 0f);
        ll1.setLabelPosition(LimitLine.LimitLabelPosition.RIGHT_TOP);

        LimitLine ll2 = new LimitLine(-30f, "Lower Limit");
        ll2.setLineWidth(4f);
        ll2.enableDashedLine(10f, 10f, 0f);
        ll2.setLabelPosition(LimitLine.LimitLabelPosition.RIGHT_BOTTOM);

        // draw limit lines behind data instead of on top
        yAxis.setDrawLimitLinesBehindData(true);
        xAxis.setDrawLimitLinesBehindData(true);

        // add limit lines
        yAxis.addLimitLine(ll1);
        yAxis.addLimitLine(ll2);
        //xAxis.addLimitLine(llXAxis);




        // add data
        setData();

        // get the legend (only possible after setting data)
        Legend l = chart.getLegend();

        // draw legend entries as lines
        l.setForm(Legend.LegendForm.LINE);

    }

    private void setData() {


        RequestQueue queue = Volley.newRequestQueue(this);
        String url ="https://iotplotter.com/api/v2/feed/829369784150976580";

        StringRequest stringRequest = new StringRequest(Request.Method.GET, url,
                response -> {
                    ArrayList<Entry> values = new ArrayList<>();

                    try {
                        JSONObject responseObject = new JSONObject(response);
                        JSONArray js = responseObject.names();  //getJSONObject("commit").getJSONObject("committer").getString("date");
                        assert js != null;
                        for (int i = 0; i < js.length(); i++) {
                            JSONObject dataObject = responseObject.getJSONObject(js.getString(i)).getJSONObject("data");
                            values.add(new Entry(js.getInt(i), (float) dataObject.getDouble("Temperatur")));

                        }
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }

                    LineDataSet set1 = (LineDataSet) chart.getData().getDataSetByIndex(0);

                    Log.e("DATA", values.toString());

                    set1.setValues(values);
                    set1.notifyDataSetChanged();
                    chart.getData().notifyDataChanged();
                    chart.notifyDataSetChanged();

                    // draw points over time
                    chart.animateY(500, Easing.EaseOutCubic);

                    Entry latestEntry = values.get(values.size() - 1);
                    Date lastUpdate = new Date((long) (latestEntry.getX() * 1000));


                    ((TextView) findViewById(R.id.last_update)).setText(DateUtils.isToday(lastUpdate.getTime()) ?
                            new SimpleDateFormat("HH:mm", Locale.GERMANY).format(lastUpdate) :
                            new SimpleDateFormat("dd.MM.yyyy", Locale.GERMANY).format(lastUpdate));
                    ((TextView) findViewById(R.id.current_value)).setText(latestEntry.getY() + " °C");


                }, error -> Toast.makeText(MainActivity.this, error.getMessage(), Toast.LENGTH_LONG).show());

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

            set1.setHighlightEnabled(false);

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