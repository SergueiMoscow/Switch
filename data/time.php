<?php
date_default_timezone_set("Europe/Moscow");
if (isset($_GET['tz']))
  echo date("T");
elseif(isset($_GET['u']))
	echo time();
elseif(isset($_GET['json']))
{
  $now = new DateTime(); // Get current DateTime
  $unixtime = $now->getTimestamp(); // Get Unix timestamp

  $data = [
    "abbreviation" => "MSK",
    "client_ip" => "46.188.82.8",
    "datetime" => $now->format('Y-m-d\TH:i:s.uP'), // Current datetime in required format
    "day_of_week" => $now->format('N'), // Day of week
    "day_of_year" => $now->format('z'), // Day of year
    "dst" => false,
    "dst_from" => null,
    "dst_offset" => 0,
    "dst_until" => null,
    "raw_offset" => 10800,
    "timezone" => "Europe/Moscow",
    "unixtime" => $unixtime, // Unix timestamp
    "utc_datetime" => gmdate('Y-m-d\TH:i:s.u', $unixtime), // Convert Unix timestamp to GMT datetime string
    "utc_offset" => "+03:00",
    "week_number" => $now->format('W'), // Week number
  ];

header('Content-Type: application/json');
echo json_encode($data); // Return JSON data
  // $retArray=array(
  //   "Y"=>date("Y"),
  //   "m"=>date("m"),
  //   "d"=>date("d"),
  //   "H"=>date("H"),
  //   "i"=>date("i"),
  //   "s"=>date("s"),
  //   "z"=>date("T")
  // );
  // echo json_encode($retArray);
}
else
  echo date("Y-m-d")."T".date("H:i:s");
?>